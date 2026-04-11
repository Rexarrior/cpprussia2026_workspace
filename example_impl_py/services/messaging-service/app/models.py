from dataclasses import dataclass, field
from typing import Optional
import uuid
from datetime import datetime


@dataclass
class Channel:
    id: int
    name: str


@dataclass
class Message:
    id: int
    channel_id: int
    user_login: str
    user_name: str
    message: str
    timestamp: str


# In-memory storage
channels_db: dict[int, Channel] = {
    1: Channel(id=1, name="general"),
    2: Channel(id=2, name="random"),
}
messages_db: dict[int, Message] = {}
message_counter: int = 0


def generate_message_id() -> int:
    global message_counter
    message_counter += 1
    return message_counter
