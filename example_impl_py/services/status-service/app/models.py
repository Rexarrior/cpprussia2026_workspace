from dataclasses import dataclass
from typing import Optional
from datetime import datetime, timedelta


@dataclass
class UserStatus:
    login: str
    status_type: str
    status_message: str
    visibility: str
    updated_at: datetime
    expires_at: Optional[datetime] = None


statuses_db: dict[str, UserStatus] = {}

DEFAULT_TTL_SECONDS = 3600
