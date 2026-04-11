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
        assert response.json()["service"] == "auth-service"


@pytest.mark.asyncio
async def test_register_and_authorize():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        reg_response = await client.post(
            "/v1/user/registration",
            json={
                "login": "testuser",
                "name": "Test User",
                "email": "test@example.com",
                "phone": "+1234567890",
                "password": "password123",
            },
        )
        assert reg_response.status_code == 200
        data = reg_response.json()
        assert "current_user" in data
        assert data["current_user"]["login"] == "testuser"
        assert len(data["current_user"]["token"]) == 128

        auth_response = await client.post(
            "/v1/user/authorization",
            json={"login": "testuser", "password": "password123"},
        )
        assert auth_response.status_code == 200
        assert auth_response.json()["current_user"]["login"] == "testuser"


@pytest.mark.asyncio
async def test_duplicate_registration():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        await client.post(
            "/v1/user/registration",
            json={
                "login": "dupuser",
                "name": "Dup User",
                "email": "dup@example.com",
                "phone": "+1234567890",
                "password": "password123",
            },
        )
        dup_response = await client.post(
            "/v1/user/registration",
            json={
                "login": "dupuser",
                "name": "Dup User 2",
                "email": "dup2@example.com",
                "phone": "+0987654321",
                "password": "password456",
            },
        )
        assert dup_response.status_code == 409


@pytest.mark.asyncio
async def test_invalid_credentials():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/user/authorization",
            json={"login": "nonexistent", "password": "wrongpass"},
        )
        assert response.status_code == 401
