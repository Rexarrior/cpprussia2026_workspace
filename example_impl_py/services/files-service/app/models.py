from dataclasses import dataclass
import base64
import uuid


@dataclass
class File:
    file_id: str
    login: str
    filename: str
    content_base64: str
    size: int
    mime_type: str
    uri: str


files_db: dict[str, File] = {}
