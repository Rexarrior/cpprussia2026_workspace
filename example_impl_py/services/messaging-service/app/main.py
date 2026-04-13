from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.router import router as messaging_router, direct_router
from app.config import Settings

settings = Settings()


@asynccontextmanager
async def lifespan(app: FastAPI):
    print(f"{settings.SERVICE_NAME} starting...")
    yield
    print(f"{settings.SERVICE_NAME} shutting down...")


app = FastAPI(
    title=settings.SERVICE_NAME,
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(messaging_router)
app.include_router(direct_router)


@app.get("/health")
async def health():
    return {"status": "healthy", "service": settings.SERVICE_NAME}
