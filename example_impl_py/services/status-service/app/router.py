from fastapi import APIRouter, HTTPException, status, Header
from datetime import datetime, timedelta, timezone
from app.schemas import (
    V1UserStatusUpdateRequest,
    V1UserStatusUpdateResponse,
    V1UserStatusByLoginRequest,
    V1UserStatusByLoginResponse,
    V1Error,
)
from app.models import statuses_db, DEFAULT_TTL_SECONDS, UserStatus

router = APIRouter(prefix="/v1/user/status", tags=["status"])

VALID_STATUS_TYPES = {"online", "away", "busy", "offline"}
VALID_VISIBILITY = {"public", "private"}


def get_current_user(authorization: str = Header(...)) -> dict:
    if len(authorization) != 128:
        raise HTTPException(
            status_code=401,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": authorization, "login": "unknown", "name": "Unknown User"}


def is_status_expired(target_status: UserStatus) -> bool:
    if target_status.expires_at is None:
        return False
    return datetime.now(timezone.utc) > target_status.expires_at


@router.post(
    "/update",
    response_model=V1UserStatusUpdateResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def update_status(
    request: V1UserStatusUpdateRequest, authorization: str = Header(...)
):
    get_current_user(authorization)

    status_type = request.status.get("status_type")
    if status_type and status_type not in VALID_STATUS_TYPES:
        raise HTTPException(
            status_code=400,
            detail={
                "code": "invalid_status_type",
                "message": f"Invalid status_type. Must be one of: {VALID_STATUS_TYPES}",
            },
        )

    visibility = request.status.get("visibility", "public")
    if visibility not in VALID_VISIBILITY:
        raise HTTPException(
            status_code=400,
            detail={
                "code": "invalid_visibility",
                "message": f"Invalid visibility. Must be one of: {VALID_VISIBILITY}",
            },
        )

    now = datetime.now(timezone.utc)
    expires_at = now + timedelta(seconds=DEFAULT_TTL_SECONDS)

    user_status = UserStatus(
        login=request.current_user.login,
        status_type=status_type or "online",
        status_message=request.status.get("status_message", ""),
        visibility=visibility,
        updated_at=now,
        expires_at=expires_at,
    )
    statuses_db[request.current_user.login] = user_status

    return V1UserStatusUpdateResponse(
        success=True,
        updated_at=now.isoformat() + "Z",
        expires_at=expires_at.isoformat() + "Z",
    )


@router.post(
    "/by-login",
    response_model=V1UserStatusByLoginResponse,
    responses={
        400: {"model": V1Error},
        401: {"model": V1Error},
        403: {"model": V1Error},
        404: {"model": V1Error},
    },
)
async def get_status_by_login(
    request: V1UserStatusByLoginRequest, authorization: str = Header(...)
):
    get_current_user(authorization)

    target_status = statuses_db.get(request.login)

    if not target_status:
        raise HTTPException(
            status_code=404,
            detail={"code": "status_not_found", "message": "User status not found"},
        )

    if is_status_expired(target_status):
        raise HTTPException(
            status_code=404,
            detail={"code": "status_expired", "message": "User status has expired"},
        )

    if (
        target_status.visibility == "private"
        and target_status.login != request.current_user.login
    ):
        raise HTTPException(
            status_code=403,
            detail={"code": "forbidden", "message": "Status is private"},
        )

    return V1UserStatusByLoginResponse(
        status={
            "status_type": target_status.status_type,
            "status_message": target_status.status_message,
            "visibility": target_status.visibility,
            "updated_at": target_status.updated_at.isoformat() + "Z",
        }
    )
