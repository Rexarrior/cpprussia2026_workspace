from pydantic import BaseModel, Field
from typing import Optional, List, Literal


class V1Login(BaseModel):
    login: str = Field(..., min_length=3)


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1ChannelId(BaseModel):
    channel_id: int


class V1MessageId(BaseModel):
    message_id: int


class V1Animation(str):
    LIKE = "like"
    DISLIKE = "dislike"
    HEART = "heart"
    FIRE = "fire"
    OKAY = "okay"
    LOL = "LOL"
    SMILE = "smile"


class V1LikeTriggerRequest(BaseModel):
    current_user: V1CurrentUser
    idempotency_token: str = Field(..., min_length=16, max_length=256)
    channel_id: int
    message_id: int
    animation: str = Field(...)


class V1LikeTriggerResponse(BaseModel):
    action: Literal["added", "removed"]
    current_user_reaction: Optional[str] = None


class V1GetReactionsResponse(BaseModel):
    reactions: List[dict]


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None
