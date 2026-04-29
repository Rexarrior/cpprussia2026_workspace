from pydantic import BaseModel, Field
from typing import Optional, List, Union


class V1Login(BaseModel):
    login: str = Field(..., min_length=3)


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1ChannelId(BaseModel):
    channel_id: Union[int, str]


class V1MessageId(BaseModel):
    message_id: int


class V1NotificationStatus(BaseModel):
    message_id: int
    read: bool = False


class V1ChannelNotificationListRequest(BaseModel):
    current_user: V1CurrentUser
    channel_id: Union[int, str]


class V1ChannelNotificationListResponse(BaseModel):
    notifications: List[V1NotificationStatus]


class V1ChannelNotificationNewRequest(BaseModel):
    current_user: V1CurrentUser
    channel_id: Union[int, str]
    message_id: int
    other_user_login: str = Field(..., min_length=3)


class V1ChannelNotificationNewResponse(BaseModel):
    notification_id: str


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None
