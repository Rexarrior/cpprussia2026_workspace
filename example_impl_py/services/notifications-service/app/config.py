from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    SERVICE_NAME: str = "notifications-service"
    HOST: str = "0.0.0.0"
    PORT: int = 8003

    model_config = SettingsConfigDict(env_file=".env")


settings = Settings()
