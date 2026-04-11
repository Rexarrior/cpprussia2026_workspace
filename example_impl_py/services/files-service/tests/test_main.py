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
        assert response.json()["service"] == "files-service"


@pytest.mark.asyncio
async def test_upload_file():
    valid_token = "a" * 128
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/file/new",
            json={
                "login": "testuser",
                "filename": "test.txt",
                "content": "dGVzdCBjb250ZW50",  # "test content" in base64
            },
            headers={"token": valid_token},
        )
        assert response.status_code == 200
        data = response.json()
        assert "uri" in data
        assert data["uri"].startswith("s3://files-bucket/")
        assert data["file"]["filename"] == "test.txt"


@pytest.mark.asyncio
async def test_download_file_owner():
    valid_token = "b" * 128
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        upload_response = await client.post(
            "/v1/file/new",
            json={
                "login": "owneruser",
                "filename": "secret.txt",
                "content": "c2VjcmV0",  # "secret" in base64
            },
            headers={"token": valid_token},
        )
        assert upload_response.status_code == 200
        uri = upload_response.json()["uri"]

        download_response = await client.post(
            "/v1/file/by-uri",
            json={
                "current_user": {
                    "token": valid_token,
                    "login": "owneruser",
                    "name": "Owner User",
                },
                "uri": uri,
            },
            headers={"token": valid_token},
        )
        assert download_response.status_code == 200
        data = download_response.json()
        assert data["login"] == "owneruser"
        assert data["filename"] == "secret.txt"
        assert data["content"] == "c2VjcmV0"


@pytest.mark.asyncio
async def test_download_file_forbidden():
    valid_token_owner = "c" * 128
    valid_token_other = "d" * 128
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        upload_response = await client.post(
            "/v1/file/new",
            json={
                "login": "fileowner",
                "filename": "private.txt",
                "content": "cHJpdmF0ZQ==",  # "private" in base64
            },
            headers={"token": valid_token_owner},
        )
        assert upload_response.status_code == 200
        uri = upload_response.json()["uri"]

        download_response = await client.post(
            "/v1/file/by-uri",
            json={
                "current_user": {
                    "token": valid_token_other,
                    "login": "otheruser",
                    "name": "Other User",
                },
                "uri": uri,
            },
            headers={"token": valid_token_other},
        )
        assert download_response.status_code == 403


@pytest.mark.asyncio
async def test_file_not_found():
    valid_token = "e" * 128
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.post(
            "/v1/file/by-uri",
            json={
                "current_user": {
                    "token": valid_token,
                    "login": "someuser",
                    "name": "Some User",
                },
                "uri": "s3://files-bucket/nonexistent-file-id",
            },
            headers={"token": valid_token},
        )
        assert response.status_code == 404
