from fastapi import APIRouter, HTTPException, status
from app.schemas import (
    V1UserRegistrationRequest,
    V1UserAuthorizationRequest,
    V1UserAuthorizationResponse,
    V1Error,
    V1CurrentUser,
)
from app.models import (
    users_db,
    tokens_db,
    hash_password,
    verify_password,
    generate_token,
)

router = APIRouter(prefix="/v1/user", tags=["auth"])


@router.post(
    "/registration",
    response_model=V1UserAuthorizationResponse,
    responses={
        400: {"model": V1Error, "description": "Validation error"},
        409: {"model": V1Error, "description": "User already exists"},
    },
)
async def register(request: V1UserRegistrationRequest):
    if request.login in users_db:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail={
                "code": "user_exists",
                "message": f"User {request.login} already exists",
            },
        )

    from app.models import User

    token = generate_token()
    user = User(
        login=request.login,
        name=request.name,
        email=request.email,
        phone=request.phone,
        password_hash=hash_password(request.password),
        token=token,
    )
    users_db[request.login] = user
    tokens_db[token] = user

    return V1UserAuthorizationResponse(
        current_user=V1AuthorizedUser(token=token, login=user.login, name=user.name)
    )


@router.post(
    "/authorization",
    response_model=V1UserAuthorizationResponse,
    responses={
        401: {"model": V1Error, "description": "Invalid credentials"},
    },
)
async def authorize(request: V1UserAuthorizationRequest):
    user = users_db.get(request.login)

    if not user or not verify_password(request.password, user.password_hash):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail={
                "code": "invalid_credentials",
                "message": "Invalid login or password",
            },
        )

    if user.token in tokens_db:
        del tokens_db[user.token]

    token = generate_token()
    user.token = token
    tokens_db[token] = user

    return V1UserAuthorizationResponse(
        current_user=V1AuthorizedUser(token=token, login=user.login, name=user.name)
    )
