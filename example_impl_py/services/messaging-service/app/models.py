from dataclasses import dataclass, field
from typing import Optional, Union
import uuid
from datetime import datetime


@dataclass
class Channel:
    id: int
    name: str


@dataclass
class DirectChannel:
    """Direct message channel between two users."""

    channel_id: str  # Format: dm_{user1}_{user2} where user1 < user2 alphabetically
    user1: str
    user2: str


@dataclass
class Message:
    id: int
    channel_id: Union[int, str]
    user_login: str
    user_name: str
    message: str
    timestamp: str


# In-memory storage
channels_db: dict[int, Channel] = {
    1: Channel(id=1, name="general"),
    2: Channel(id=2, name="random"),
}
# Direct channels: channel_id -> DirectChannel
direct_channels_db: dict[str, DirectChannel] = {}
messages_db: dict[int, Message] = {}
message_counter: int = 0


def generate_message_id() -> int:
    global message_counter
    message_counter += 1
    return message_counter


def get_direct_channel_id(user1: str, user2: str) -> str:
    """Generate deterministic channel ID for direct messages."""
    if user1 < user2:
        return f"dm_{user1}_{user2}"
    else:
        return f"dm_{user2}_{user1}"
