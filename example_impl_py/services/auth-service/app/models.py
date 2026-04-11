from dataclasses import dataclass, field
from typing import Optional
import uuid
import hashlib


@dataclass
class User:
    login: str
    name: str
    email: str
    phone: str
    password_hash: str
    token: Optional[str] = None


users_db: dict[str, User] = {}
tokens_db: dict[str, User] = {}


def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode()).hexdigest()


def verify_password(password: str, password_hash: str) -> bool:
    return hash_password(password) == password_hash


def generate_token() -> str:
    # 4 UUIDs = 32*4 = 128 hex characters
    return (
        str(uuid.uuid4()).replace("-", "")
        + str(uuid.uuid4()).replace("-", "")
        + str(uuid.uuid4()).replace("-", "")
        + str(uuid.uuid4()).replace("-", "")
    )
