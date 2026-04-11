from fastapi import APIRouter, HTTPException, status, Header, Path
from typing import Optional, List, Dict
from collections import defaultdict
import uuid
from app.schemas import (
    V1LikeTriggerRequest,
    V1LikeTriggerResponse,
    V1GetReactionsResponse,
    V1Error,
)
from app.models import reactions_db, Reaction

router = APIRouter(prefix="/v1/like", tags=["reactions"])

VALID_ANIMATIONS = {"like", "dislike", "heart", "fire", "okay", "LOL", "smile"}


def get_current_user(token: str = Header(...)) -> dict:
    if len(token) != 128:
        raise HTTPException(
            status_code=401,
            detail={"code": "invalid_token", "message": "Invalid token"},
        )
    return {"token": token, "login": "unknown", "name": "Unknown User"}


@router.post(
    "/trigger",
    response_model=V1LikeTriggerResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def trigger_reaction(
    request: V1LikeTriggerRequest,
    token: str = Header(...),
):
    user = get_current_user(token)

    if request.animation not in VALID_ANIMATIONS:
        raise HTTPException(
            status_code=400,
            detail={"code": "invalid_animation", "message": "Invalid animation type"},
        )

    key = (request.channel_id, request.message_id, request.current_user.login)

    existing = reactions_db.get(key)

    if existing:
        if existing.idempotency_token == request.idempotency_token:
            return V1LikeTriggerResponse(
                action="removed"
                if existing.animation == request.animation
                else "added",
                current_user_reaction=request.animation,
            )
        else:
            del reactions_db[key]

    reaction = Reaction(
        id=str(uuid.uuid4()),
        channel_id=request.channel_id,
        message_id=request.message_id,
        user_login=request.current_user.login,
        animation=request.animation,
        idempotency_token=request.idempotency_token,
    )
    reactions_db[key] = reaction

    return V1LikeTriggerResponse(
        action="added", current_user_reaction=request.animation
    )


@router.get(
    "/{channel_id}/{message_id}",
    response_model=V1GetReactionsResponse,
    responses={400: {"model": V1Error}, 401: {"model": V1Error}},
)
async def get_reactions(
    channel_id: int = Path(...),
    message_id: int = Path(...),
    token: str = Header(...),
):
    user = get_current_user(token)

    animation_counts: Dict[str, List[str]] = defaultdict(list)

    for (c_id, m_id, u_login), reaction in reactions_db.items():
        if c_id == channel_id and m_id == message_id:
            animation_counts[reaction.animation].append(reaction.user_login)

    reactions = [
        {"animation": anim, "count": len(logins), "user_logins": logins}
        for anim, logins in animation_counts.items()
    ]

    return V1GetReactionsResponse(reactions=reactions)
