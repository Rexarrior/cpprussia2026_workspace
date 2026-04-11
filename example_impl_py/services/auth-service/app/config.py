from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    SERVICE_NAME: str = "auth-service"
    HOST: str = "0.0.0.0"
    PORT: int = 8001
    SECRET_KEY: str = "workshop-secret-key-change-in-prod"

    class Config:
        env_file = ".env"
