from pydantic import BaseModel, Field
from typing import Optional, Dict, Any


class V1Login(BaseModel):
    login: str = Field(..., min_length=3)


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1StatusType(str):
    ONLINE = "online"
    AWAY = "away"
    BUSY = "busy"
    OFFLINE = "offline"


class V1Visibility(str):
    PUBLIC = "public"
    PRIVATE = "private"


class V1UserStatus(BaseModel):
    status_type: Optional[str] = None
    status_message: Optional[str] = None
    visibility: Optional[str] = None
    extra: Optional[Dict[str, Any]] = None


class V1UserStatusUpdateRequest(BaseModel):
    current_user: V1CurrentUser
    status: dict


class V1UserStatusUpdateResponse(BaseModel):
    success: bool
    updated_at: str
    expires_at: Optional[str] = None


class V1UserStatusByLoginRequest(BaseModel):
    current_user: V1CurrentUser
    login: str


class V1UserStatusByLoginResponse(BaseModel):
    status: dict


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None
