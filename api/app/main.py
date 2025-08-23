from contextlib import asynccontextmanager
from fastapi import FastAPI
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("startup")
    yield
    logger.info("shutdown")

app = FastAPI(lifespan=lifespan)

@app.get("/healthz")
async def healthz():
    return {"OK": True}