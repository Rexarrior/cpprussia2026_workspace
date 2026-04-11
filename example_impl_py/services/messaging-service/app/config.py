from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    SERVICE_NAME: str = "messaging-service"
    HOST: str = "0.0.0.0"
    PORT: int = 8002

    model_config = SettingsConfigDict(env_file=".env")
