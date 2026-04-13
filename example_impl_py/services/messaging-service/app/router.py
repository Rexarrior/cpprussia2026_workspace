from fastapi import APIRouter, HTTPException, status, Header
from typing import Optional, Union
from datetime import datetime, timezone
from app.schemas import (
    V1ChannelMessageByTimestampRequest,
    V1ChannelMessageByTimestampResponse,
    V1ChannelMessageNewRequest,
    V1ChannelMessageNewResponse,
    V1ChannelMessage,
    V1Error,
    V1CurrentUser,
    V1DirectGetOrCreateRequest,
    V1DirectGetOrCreateResponse,
    V1DirectListRequest,
    V1DirectListResponse,
    V1DirectChatInfo,
    V1PublicUser,
)
from app.models import (
    channels_db,
    direct_channels_db,
    messages_db,
    generate_message_id,
    get_direct_channel_id,
    DirectChannel,
)

router = APIRouter(prefix="/v1/channel", tags=["messaging"])


def get_current_user(authorization: str = Header(...)) -> dict:
    """Validate token and return user info. In production, call auth-service."""
    if len(authorization) != 128:
        raise HTTPException(
            status_code=401,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": authorization, "login": "unknown", "name": "Unknown User"}


def channel_exists(channel_id: Union[int, str]) -> bool:
    """Check if a channel (regular or direct) exists."""
    if isinstance(channel_id, int):
        return channel_id in channels_db
    else:
        return channel_id in direct_channels_db


@router.post(
    "/message/by-timestamp",
    response_model=V1ChannelMessageByTimestampResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def get_messages_by_timestamp(
    request: V1ChannelMessageByTimestampRequest,
    authorization: str = Header(...),
):
    user = get_current_user(authorization)

    if not channel_exists(request.channel_id):
        raise HTTPException(
            status_code=404,
            detail={"code": "channel_not_found", "message": "Channel not found"},
        )

    messages = [
        V1ChannelMessage(
            current_user=V1CurrentUser(
                token=user["token"], login=msg.user_login, name=msg.user_name
            ),
            id=msg.id,
            timestamp=msg.timestamp,
            message=msg.message,
        )
        for msg in messages_db.values()
        if msg.channel_id == request.channel_id
        and msg.timestamp >= request.from_
        and (request.to is None or msg.timestamp <= request.to)
    ]
    messages.sort(key=lambda m: m.timestamp)

    return V1ChannelMessageByTimestampResponse(messages=messages, has_more=False)


@router.post(
    "/message/new",
    response_model=V1ChannelMessageNewResponse,
    responses={
        400: {"model": V1Error},
        401: {"model": V1Error},
        404: {"model": V1Error},
    },
)
async def create_message(
    request: V1ChannelMessageNewRequest,
    authorization: str = Header(...),
):
    user = get_current_user(authorization)

    if not channel_exists(request.channel_id):
        raise HTTPException(
            status_code=404,
            detail={"code": "channel_not_found", "message": "Channel not found"},
        )

    msg_id = generate_message_id()
    timestamp = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

    from app.models import Message

    msg = Message(
        id=msg_id,
        channel_id=request.channel_id,
        user_login=request.current_user.login,
        user_name=request.current_user.name,
        message=request.message,
        timestamp=timestamp,
    )
    messages_db[msg_id] = msg

    return V1ChannelMessageNewResponse(message_id=msg_id)


# Direct message endpoints
direct_router = APIRouter(prefix="/v1/direct", tags=["direct-messaging"])


@direct_router.post(
    "/get-or-create",
    response_model=V1DirectGetOrCreateResponse,
    responses={
        400: {"model": V1Error},
        401: {"model": V1Error},
    },
)
async def get_or_create_direct_chat(
    request: V1DirectGetOrCreateRequest,
    authorization: str = Header(...),
):
    """Get or create a direct message channel with another user."""
    user = get_current_user(authorization)

    current_login = request.current_user.login
    other_login = request.other_user_login

    if current_login == other_login:
        raise HTTPException(
            status_code=400,
            detail={
                "code": "invalid_request",
                "message": "Cannot create DM with yourself",
            },
        )

    # Generate channel ID
    channel_id = get_direct_channel_id(current_login, other_login)

    # Create channel if it doesn't exist
    if channel_id not in direct_channels_db:
        direct_channels_db[channel_id] = DirectChannel(
            channel_id=channel_id,
            user1=min(current_login, other_login),
            user2=max(current_login, other_login),
        )

    # Get other user info (in real app, would call auth-service)
    # For now, we'll use the login as name since we don't have access to user data
    other_user = V1PublicUser(login=other_login, name=other_login)

    return V1DirectGetOrCreateResponse(
        channel_id=channel_id,
        other_user=other_user,
    )


@direct_router.post(
    "/list",
    response_model=V1DirectListResponse,
    responses={
        400: {"model": V1Error},
        401: {"model": V1Error},
    },
)
async def list_direct_chats(
    request: V1DirectListRequest,
    authorization: str = Header(...),
):
    """List all direct message channels for the current user."""
    user = get_current_user(authorization)

    current_login = request.current_user.login
    direct_chats = []

    for channel in direct_channels_db.values():
        if channel.user1 == current_login or channel.user2 == current_login:
            # Determine the other user
            other_login = (
                channel.user2 if channel.user1 == current_login else channel.user1
            )
            other_user = V1PublicUser(login=other_login, name=other_login)

            # Get last message in this channel
            channel_messages = [
                msg
                for msg in messages_db.values()
                if msg.channel_id == channel.channel_id
            ]
            last_message = None
            if channel_messages:
                latest = max(channel_messages, key=lambda m: m.timestamp)
                last_message = V1ChannelMessage(
                    current_user=V1CurrentUser(
                        token=user["token"],
                        login=latest.user_login,
                        name=latest.user_name,
                    ),
                    id=latest.id,
                    timestamp=latest.timestamp,
                    message=latest.message,
                )

            direct_chats.append(
                V1DirectChatInfo(
                    channel_id=channel.channel_id,
                    other_user=other_user,
                    last_message=last_message,
                )
            )

    return V1DirectListResponse(direct_chats=direct_chats)
