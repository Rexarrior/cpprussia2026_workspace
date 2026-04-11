from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    SERVICE_NAME: str = "reactions-service"
    PORT: int = 8005


settings = Settings()
