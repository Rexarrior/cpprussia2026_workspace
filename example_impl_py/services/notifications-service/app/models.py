from dataclasses import dataclass
from typing import Optional
import uuid


@dataclass
class Notification:
    notification_id: str
    channel_id: int
    message_id: int
    from_user_login: str
    to_user_login: str
    read: bool = False


notifications_db: dict[str, Notification] = {}
