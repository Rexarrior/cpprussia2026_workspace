import pytest
from fastapi.testclient import TestClient
from httpx import ASGITransport, AsyncClient
from app.main import app
from app.models import reactions_db


@pytest.fixture(autouse=True)
def clear_db():
    reactions_db.clear()
    yield
    reactions_db.clear()


@pytest.fixture
def client():
    return TestClient(app)


@pytest.fixture
def valid_token():
    return "a" * 128


@pytest.fixture
def valid_user():
    return {"token": "a" * 128, "login": "testuser", "name": "Test User"}


def test_health_check(client):
    response = client.get("/health")
    assert response.status_code == 200
    assert response.json() == {"status": "healthy", "service": "reactions-service"}


def test_trigger_reaction_add(client, valid_token, valid_user):
    response = client.post(
        "/v1/like/trigger",
        json={
            "current_user": valid_user,
            "idempotency_token": "token12345678901234",
            "channel_id": 1,
            "message_id": 100,
            "animation": "like",
        },
        headers={"token": valid_token},
    )
    assert response.status_code == 200
    data = response.json()
    assert data["action"] == "added"
    assert data["current_user_reaction"] == "like"


def test_trigger_reaction_same_token_idempotent(client, valid_token, valid_user):
    client.post(
        "/v1/like/trigger",
        json={
            "current_user": valid_user,
            "idempotency_token": "token12345678901234",
            "channel_id": 1,
            "message_id": 100,
            "animation": "like",
        },
        headers={"token": valid_token},
    )

    response = client.post(
        "/v1/like/trigger",
        json={
            "current_user": valid_user,
            "idempotency_token": "token12345678901234",
            "channel_id": 1,
            "message_id": 100,
            "animation": "like",
        },
        headers={"token": valid_token},
    )
    assert response.status_code == 200
    data = response.json()
    assert data["action"] == "removed"
    assert data["current_user_reaction"] == "like"


def test_get_reactions(client, valid_token, valid_user):
    client.post(
        "/v1/like/trigger",
        json={
            "current_user": valid_user,
            "idempotency_token": "token12345678901234",
            "channel_id": 1,
            "message_id": 100,
            "animation": "like",
        },
        headers={"token": valid_token},
    )

    response = client.get("/v1/like/1/100", headers={"token": valid_token})
    assert response.status_code == 200
    data = response.json()
    assert len(data["reactions"]) == 1
    assert data["reactions"][0]["animation"] == "like"
    assert data["reactions"][0]["count"] == 1
    assert "testuser" in data["reactions"][0]["user_logins"]


def test_invalid_animation_error(client, valid_token, valid_user):
    response = client.post(
        "/v1/like/trigger",
        json={
            "current_user": valid_user,
            "idempotency_token": "token12345678901234",
            "channel_id": 1,
            "message_id": 100,
            "animation": "invalid_animation",
        },
        headers={"token": valid_token},
    )
    assert response.status_code == 400
    data = response.json()
    assert data["detail"]["code"] == "invalid_animation"
