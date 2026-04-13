from pydantic import BaseModel, Field, ConfigDict
from typing import Optional, List, Union
from datetime import datetime


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1ChannelId(BaseModel):
    channel_id: int


class V1MessageId(BaseModel):
    message_id: int


class V1ChannelMessage(BaseModel):
    current_user: V1CurrentUser
    id: int
    timestamp: str
    message: str


class V1ChannelMessageByTimestampRequest(BaseModel):
    model_config = ConfigDict(populate_by_name=True)

    channel_id: Union[int, str]
    from_: str = Field(..., alias="from")
    to: Optional[str] = None


class V1ChannelMessageByTimestampResponse(BaseModel):
    messages: List[V1ChannelMessage]
    next_cursor: Optional[str] = None
    has_more: bool = False


class V1ChannelMessageNewRequest(BaseModel):
    current_user: V1CurrentUser
    channel_id: Union[int, str]
    message: str = Field(..., min_length=1)


class V1ChannelMessageNewResponse(BaseModel):
    message_id: int


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None


# Direct message schemas
class V1PublicUser(BaseModel):
    login: str
    name: str


class V1DirectGetOrCreateRequest(BaseModel):
    current_user: V1CurrentUser
    other_user_login: str


class V1DirectGetOrCreateResponse(BaseModel):
    channel_id: str
    other_user: V1PublicUser


class V1DirectListRequest(BaseModel):
    current_user: V1CurrentUser


class V1DirectChatInfo(BaseModel):
    channel_id: str
    other_user: V1PublicUser
    last_message: Optional[V1ChannelMessage] = None


class V1DirectListResponse(BaseModel):
    direct_chats: List[V1DirectChatInfo]
