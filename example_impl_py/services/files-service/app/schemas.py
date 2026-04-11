from pydantic import BaseModel, Field
from typing import Optional


class V1Login(BaseModel):
    login: str = Field(..., min_length=3)


class V1CurrentUser(BaseModel):
    token: str = Field(..., min_length=128, max_length=128)
    login: str
    name: str


class V1File(BaseModel):
    login: str
    filename: str = Field(..., min_length=3, max_length=256)
    content: str


class V1FileNewRequest(V1File):
    pass


class V1FileNewResponse(BaseModel):
    current_user: V1CurrentUser
    uri: str
    file: V1File


class V1FileByUriRequest(BaseModel):
    current_user: V1CurrentUser
    uri: str = Field(..., min_length=3)


class V1FileByUriResponse(BaseModel):
    login: str
    filename: str
    content: str


class V1Error(BaseModel):
    code: str
    message: str
    details: Optional[dict] = None
