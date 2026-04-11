import pytest
from httpx import AsyncClient, ASGITransport
from app.main import app
from app.models import notifications_db


@pytest.fixture(autouse=True)
def clear_db():
    notifications_db.clear()
    yield
    notifications_db.clear()


@pytest.mark.asyncio
async def test_health():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.get("/health")
        assert response.status_code == 200
        assert response.json()["service"] == "notifications-service"


@pytest.mark.asyncio
async def test_create_notification():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "a" * 128
        response = await client.post(
            "/v1/channel/notification/new",
            json={
                "current_user": {
                    "token": token,
                    "login": "sender",
                    "name": "Sender User",
                },
                "channel_id": 1,
                "message_id": 100,
                "other_user_login": "recipient",
            },
            headers={"token": token},
        )
        assert response.status_code == 200
        data = response.json()
        assert "notification_id" in data
        assert len(data["notification_id"]) == 36


@pytest.mark.asyncio
async def test_list_notifications():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "b" * 128
        await client.post(
            "/v1/channel/notification/new",
            json={
                "current_user": {
                    "token": token,
                    "login": "sender",
                    "name": "Sender User",
                },
                "channel_id": 1,
                "message_id": 100,
                "other_user_login": "recipient",
            },
            headers={"token": token},
        )
        list_response = await client.post(
            "/v1/channel/notification/list",
            json={
                "current_user": {
                    "token": "c" * 128,
                    "login": "recipient",
                    "name": "Recipient User",
                },
                "channel_id": 1,
            },
            headers={"token": "c" * 128},
        )
        assert list_response.status_code == 200
        data = list_response.json()
        assert "notifications" in data
        assert len(data["notifications"]) == 1
        assert data["notifications"][0]["message_id"] == 100
        assert data["notifications"][0]["read"] is False


@pytest.mark.asyncio
async def test_list_notifications_empty():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "d" * 128
        list_response = await client.post(
            "/v1/channel/notification/list",
            json={
                "current_user": {
                    "token": token,
                    "login": "user_with_no_notifications",
                    "name": "No Notifications User",
                },
                "channel_id": 999,
            },
            headers={"token": token},
        )
        assert list_response.status_code == 200
        data = list_response.json()
        assert "notifications" in data
        assert len(data["notifications"]) == 0


@pytest.mark.asyncio
async def test_missing_header_token():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        token = "e" * 128
        response = await client.post(
            "/v1/channel/notification/list",
            json={
                "current_user": {"token": token, "login": "user", "name": "User"},
                "channel_id": 1,
            },
        )
        assert response.status_code == 422
