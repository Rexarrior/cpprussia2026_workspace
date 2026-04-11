from fastapi import APIRouter, HTTPException, status, Header
import uuid
from app.schemas import (
    V1ChannelNotificationListRequest,
    V1ChannelNotificationListResponse,
    V1ChannelNotificationNewRequest,
    V1ChannelNotificationNewResponse,
    V1NotificationStatus,
    V1Error,
)
from app.models import notifications_db

router = APIRouter(prefix="/v1/channel/notification", tags=["notifications"])


def get_current_user(authorization: str = Header(...)) -> dict:
    if len(authorization) != 128:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": authorization, "login": "unknown", "name": "Unknown User"}


@router.post(
    "/list",
    response_model=V1ChannelNotificationListResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def list_notifications(
    request: V1ChannelNotificationListRequest,
    authorization: str = Header(...),
):
    user = get_current_user(authorization)

    user_notifications = [
        V1NotificationStatus(message_id=n.message_id, read=n.read)
        for n in notifications_db.values()
        if n.to_user_login == request.current_user.login
        and n.channel_id == request.channel_id
    ]

    return V1ChannelNotificationListResponse(notifications=user_notifications)


@router.post(
    "/new",
    response_model=V1ChannelNotificationNewResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def create_notification(
    request: V1ChannelNotificationNewRequest,
    authorization: str = Header(...),
):
    user = get_current_user(authorization)

    notification_id = str(uuid.uuid4())
    from app.models import Notification

    notification = Notification(
        notification_id=notification_id,
        channel_id=request.channel_id,
        message_id=request.message_id,
        from_user_login=request.current_user.login,
        to_user_login=request.other_user_login,
    )
    notifications_db[notification_id] = notification

    return V1ChannelNotificationNewResponse(notification_id=notification_id)
