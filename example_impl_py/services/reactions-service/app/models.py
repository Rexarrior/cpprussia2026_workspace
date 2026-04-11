from dataclasses import dataclass
from typing import Optional
import uuid


@dataclass
class Reaction:
    id: str
    channel_id: int
    message_id: int
    user_login: str
    animation: str
    idempotency_token: str


reactions_db: dict = {}
