from pydantic import BaseModel, Field, ConfigDict
from typing import Optional, List
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

    channel_id: int
    from_: str = Field(..., alias="from")
    to: Optional[str] = None


class V1ChannelMessageByTimestampResponse(BaseModel):
    messages: List[V1ChannelMessage]
    next_cursor: Optional[str] = None
    has_more: bool = False


class V1ChannelMessageNewRequest(BaseModel):
    current_user: V1CurrentUser
    channel_id: int
    message: str = Field(..., min_length=1)


class V1ChannelMessageNewResponse(BaseModel):
    message_id: int


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None
