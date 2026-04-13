from pydantic import BaseModel, EmailStr, Field
from typing import Optional


class V1Login(BaseModel):
    login: str = Field(..., min_length=3)


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1AuthorizedUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1UserRegistrationRequest(BaseModel):
    login: str = Field(..., min_length=3)
    name: str = Field(..., min_length=1)
    email: str = Field(..., min_length=3)
    phone: str = Field(..., min_length=3)
    password: str = Field(..., min_length=6)


class V1UserAuthorizationRequest(BaseModel):
    login: str = Field(..., min_length=3)
    password: str = Field(..., min_length=6)


class V1UserAuthorizationResponse(BaseModel):
    current_user: V1AuthorizedUser


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None


# User search schemas
class V1PublicUser(BaseModel):
    login: str
    name: str


class V1UserSearchRequest(BaseModel):
    query: str = Field(..., min_length=1)
    limit: int = Field(default=10, ge=1, le=50)


class V1UserSearchResponse(BaseModel):
    users: list[V1PublicUser]
