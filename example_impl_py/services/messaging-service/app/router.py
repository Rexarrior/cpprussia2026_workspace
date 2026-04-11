from fastapi import APIRouter, HTTPException, status, Header
from typing import Optional
from datetime import datetime, timezone
from app.schemas import (
    V1ChannelMessageByTimestampRequest,
    V1ChannelMessageByTimestampResponse,
    V1ChannelMessageNewRequest,
    V1ChannelMessageNewResponse,
    V1ChannelMessage,
    V1Error,
    V1CurrentUser,
)
from app.models import channels_db, messages_db, generate_message_id

router = APIRouter(prefix="/v1/channel", tags=["messaging"])


def get_current_user(authorization: str = Header(...)) -> dict:
    """Validate token and return user info. In production, call auth-service."""
    if len(authorization) != 128:
        raise HTTPException(
            status_code=401,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": authorization, "login": "unknown", "name": "Unknown User"}


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

    if request.channel_id not in channels_db:
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

    if request.channel_id not in channels_db:
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
