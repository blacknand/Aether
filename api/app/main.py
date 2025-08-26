from contextlib import asynccontextmanager
from fastapi import FastAPI
from app.services.grpc_client import get_client
import logging

logger =  logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

@asynccontextmanager
async def lifespan(app: FastAPI):
    client = get_client("localhost:50051")
    logger.info("[PyAPI] establishing connection to Aether gRPC server...")
    await client.connect()
    logger.info("[PyAPI] gRPC connected")
    try:
        yield
    finally: 
        logger.info("[PyAPI] disconnecting from Aether gRPC server...")
        await client.close()

app = FastAPI(lifespan=lifespan)

@app.get("/healthz")
async def healthz():
    client = get_client("localhost:50051")
    return {"grpc_connected": client.is_connected}