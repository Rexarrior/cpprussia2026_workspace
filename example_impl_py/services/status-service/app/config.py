from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    SERVICE_NAME: str = "status-service"
    HOST: str = "0.0.0.0"
    PORT: int = 8006

    model_config = SettingsConfigDict(env_file=".env")


settings = Settings()
