import pytest
from httpx import AsyncClient, ASGITransport
from app.main import app


@pytest.mark.asyncio
async def test_health():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.get("/health")
        assert response.status_code == 200
        assert response.json()["service"] == "messaging-service"


@pytest.mark.asyncio
async def test_create_message():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "a" * 128
        response = await client.post(
            "/v1/channel/message/new",
            json={
                "current_user": {
                    "token": token,
                    "login": "testuser",
                    "name": "Test User",
                },
                "channel_id": 1,
                "message": "Hello, world!",
            },
            headers={"token": token},
        )
        assert response.status_code == 200
        data = response.json()
        assert "message_id" in data
        assert data["message_id"] > 0


@pytest.mark.asyncio
async def test_get_messages_by_timestamp():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "a" * 128

        await client.post(
            "/v1/channel/message/new",
            json={
                "current_user": {
                    "token": token,
                    "login": "testuser",
                    "name": "Test User",
                },
                "channel_id": 1,
                "message": "Test message",
            },
            headers={"token": token},
        )

        response = await client.post(
            "/v1/channel/message/by-timestamp",
            json={
                "channel_id": 1,
                "from": "2020-01-01T00:00:00Z",
            },
            headers={"token": token},
        )
        assert response.status_code == 200
        data = response.json()
        assert "messages" in data
        assert isinstance(data["messages"], list)


@pytest.mark.asyncio
async def test_channel_not_found():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "a" * 128
        response = await client.post(
            "/v1/channel/message/new",
            json={
                "current_user": {
                    "token": token,
                    "login": "testuser",
                    "name": "Test User",
                },
                "channel_id": 999,
                "message": "Hello!",
            },
            headers={"token": token},
        )
        assert response.status_code == 404


@pytest.mark.asyncio
async def test_invalid_token():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        valid_token = "a" * 128
        response = await client.post(
            "/v1/channel/message/new",
            json={
                "current_user": {
                    "token": valid_token,
                    "login": "testuser",
                    "name": "Test User",
                },
                "channel_id": 1,
                "message": "Hello!",
            },
            headers={"token": "short"},
        )
        assert response.status_code == 401
