from fastapi import APIRouter, HTTPException, status, Header
from app.schemas import (
    V1FileNewRequest,
    V1FileNewResponse,
    V1FileByUriRequest,
    V1FileByUriResponse,
    V1CurrentUser,
    V1Error,
    V1File,
)
from app.models import files_db, File
import base64
import uuid

router = APIRouter(prefix="/v1/file", tags=["files"])


def get_current_user(token: str = Header(...)) -> dict:
    if len(token) != 128:
        raise HTTPException(
            status_code=401,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": token, "login": "unknown", "name": "Unknown User"}


def detect_mime_type(content_base64: str, filename: str) -> str:
    ext = filename.split(".")[-1].lower() if "." in filename else ""
    mime_map = {
        "jpg": "image/jpeg",
        "jpeg": "image/jpeg",
        "png": "image/png",
        "gif": "image/gif",
        "pdf": "application/pdf",
        "txt": "text/plain",
        "json": "application/json",
        "html": "text/html",
        "css": "text/css",
        "js": "application/javascript",
    }
    return mime_map.get(ext, "application/octet-stream")


@router.post(
    "/new",
    response_model=V1FileNewResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def upload_file(
    request: V1FileNewRequest,
    token: str = Header(...),
):
    user = get_current_user(token)

    file_id = str(uuid.uuid4())
    uri = f"s3://files-bucket/{file_id}"
    size = len(base64.b64decode(request.content)) if request.content else 0
    mime_type = detect_mime_type(request.content, request.filename)

    file = File(
        file_id=file_id,
        login=request.login,
        filename=request.filename,
        content_base64=request.content,
        size=size,
        mime_type=mime_type,
        uri=uri,
    )
    files_db[uri] = file

    return V1FileNewResponse(
        current_user=V1CurrentUser(
            token=user["token"], login=user["login"], name=user["name"]
        ),
        uri=uri,
        file=V1File(
            login=request.login, filename=request.filename, content=request.content
        ),
    )


@router.post(
    "/by-uri",
    response_model=V1FileByUriResponse,
    responses={
        400: {"model": V1Error},
        401: {"model": V1Error},
        403: {"model": V1Error},
        404: {"model": V1Error},
    },
)
async def get_file(
    request: V1FileByUriRequest,
    token: str = Header(...),
):
    user = get_current_user(token)

    file = files_db.get(request.uri)
    if not file:
        raise HTTPException(
            status_code=404,
            detail={"code": "file_not_found", "message": "File not found"},
        )

    if file.login != request.current_user.login:
        raise HTTPException(
            status_code=403,
            detail={
                "code": "forbidden",
                "message": "Not authorized to download this file",
            },
        )

    return V1FileByUriResponse(
        login=file.login,
        filename=file.filename,
        content=file.content_base64,
    )
