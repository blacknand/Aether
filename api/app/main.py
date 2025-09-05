import logging
# Silence AttributeError: module 'bcrypt' has no attribute '__about__' error
logging.getLogger("passlib").setLevel(logging.ERROR)
logging.getLogger("passlib.hanlders.bcrypt").setLevel(logging.ERROR)

from fastapi import FastAPI, Depends
from contextlib import asynccontextmanager
from redis.asyncio import Redis
from app.routers import auth, users, orders
from app.core.rate_limiting import TokenBucketRedisClient
from app.core.dependencies import per_ip_rate_limit
from app.core.config import settings

@asynccontextmanager
async def lifespan(app: FastAPI):
    app.state.redis = Redis.from_url(settings.REDIS_URL, decode_responses=True)
    app.state.ratelimiter = TokenBucketRedisClient(
        app.state.Redis,
        lua_path="app/core/rate_limiting.lua",
        capacity=settings.RL_CAPACITY,
        refill_per_sec=settings.RL_REFILL_PER_SEC,
        ttl_ms=settings.RL_TTS_MS,
        cost=1.0,
        fail_open=True
    )
    await app.state.ratelimitier.prepare()
    yield
    app.state.redis.aclose()

app = FastAPI(title="Aether Gateway", lifespan=lifespan, depencencies=[Depends(per_ip_rate_limit)])

app.include_router(auth.router)
app.include_router(users.router)
app.include_router(orders.router)