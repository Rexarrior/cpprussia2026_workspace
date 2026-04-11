import pytest
from httpx import AsyncClient, ASGITransport
from app.main import app
from app.models import statuses_db, UserStatus
from datetime import datetime, timedelta, timezone


@pytest.fixture(autouse=True)
def clear_db():
    statuses_db.clear()
    yield
    statuses_db.clear()


def make_token():
    return "a" * 128


@pytest.mark.asyncio
async def test_health():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.get("/health")
        assert response.status_code == 200
        assert response.json()["service"] == "status-service"


@pytest.mark.asyncio
async def test_update_status():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/update",
            json={
                "current_user": {
                    "token": make_token(),
                    "login": "john_doe",
                    "name": "John Doe",
                },
                "status": {
                    "status_type": "online",
                    "status_message": "Working from home",
                    "visibility": "public",
                },
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["success"] is True
        assert "updated_at" in data
        assert "expires_at" in data


@pytest.mark.asyncio
async def test_get_status_by_login_public():
    token = make_token()
    statuses_db["john_doe"] = UserStatus(
        login="john_doe",
        status_type="away",
        status_message="In a meeting",
        visibility="public",
        updated_at=datetime.now(timezone.utc),
        expires_at=datetime.now(timezone.utc) + timedelta(hours=1),
    )

    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/by-login",
            json={
                "current_user": {
                    "token": token,
                    "login": "jane_doe",
                    "name": "Jane Doe",
                },
                "login": "john_doe",
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"]["status_type"] == "away"
        assert data["status"]["status_message"] == "In a meeting"
        assert data["status"]["visibility"] == "public"


@pytest.mark.asyncio
async def test_get_status_by_login_private_forbidden():
    token = make_token()
    statuses_db["john_doe"] = UserStatus(
        login="john_doe",
        status_type="busy",
        status_message="Do not disturb",
        visibility="private",
        updated_at=datetime.now(timezone.utc),
        expires_at=datetime.now(timezone.utc) + timedelta(hours=1),
    )

    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/by-login",
            json={
                "current_user": {
                    "token": token,
                    "login": "jane_doe",
                    "name": "Jane Doe",
                },
                "login": "john_doe",
            },
        )
        assert response.status_code == 403
        assert response.json()["detail"]["code"] == "forbidden"


@pytest.mark.asyncio
async def test_get_status_by_login_private_owner():
    token = make_token()
    statuses_db["john_doe"] = UserStatus(
        login="john_doe",
        status_type="busy",
        status_message="Do not disturb",
        visibility="private",
        updated_at=datetime.now(timezone.utc),
        expires_at=datetime.now(timezone.utc) + timedelta(hours=1),
    )

    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/by-login",
            json={
                "current_user": {
                    "token": token,
                    "login": "john_doe",
                    "name": "John Doe",
                },
                "login": "john_doe",
            },
        )
        assert response.status_code == 200
        assert response.json()["status"]["visibility"] == "private"


@pytest.mark.asyncio
async def test_status_not_found():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/by-login",
            json={
                "current_user": {
                    "token": make_token(),
                    "login": "jane_doe",
                    "name": "Jane Doe",
                },
                "login": "nonexistent_user",
            },
        )
        assert response.status_code == 404
        assert response.json()["detail"]["code"] == "status_not_found"


@pytest.mark.asyncio
async def test_invalid_status_type():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/update",
            json={
                "current_user": {
                    "token": make_token(),
                    "login": "john_doe",
                    "name": "John Doe",
                },
                "status": {
                    "status_type": "invalid_type",
                    "status_message": "Test",
                    "visibility": "public",
                },
            },
        )
        assert response.status_code == 400
        assert response.json()["detail"]["code"] == "invalid_status_type"


@pytest.mark.asyncio
async def test_status_expired():
    token = make_token()
    statuses_db["john_doe"] = UserStatus(
        login="john_doe",
        status_type="online",
        status_message="Was here",
        visibility="public",
        updated_at=datetime.now(timezone.utc) - timedelta(hours=2),
        expires_at=datetime.now(timezone.utc) - timedelta(hours=1),
    )

    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/status/by-login",
            json={
                "current_user": {
                    "token": token,
                    "login": "jane_doe",
                    "name": "Jane Doe",
                },
                "login": "john_doe",
            },
        )
        assert response.status_code == 404
        assert response.json()["detail"]["code"] == "status_expired"
