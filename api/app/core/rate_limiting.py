# api/app/core/dependencies.py

from __future__ import annotations
from math import ceil
from typing import Callable, Optional
from fastapi import Request, Response, HTTPException, status
from redis.asyncio import Redis
from redis.exceptions import ResponseError


class TokenBucketRedisClient:
    def __init__(self, redis: Redis, 
                    lua_path: str, 
                    *, 
                    capacity: int, 
                    refill_per_sec: float, 
                    ttl_ms: int, 
                    cost: float = 1.0, 
                    fail_open: bool = True):
        self.redis = redis
        self.lua_source = Path(lua_path).read_text()
        self.sha = None
        self.capacity = capacity
        self.refill_per_ms = refill_per_sec / 1000.0
        self.ttl_ms = ttl_ms
        self.cost = cost
        self.fail_open = fail_open

    async def prepare(self, key):
        self.sha = await self.redis.script_load(self.lua_source)

    async def call(self, key: str):
        try:
            allowed, retry_ms, remaining = await self.redis.evalsha(
                self.sha, 1, key,
                str(self.capacity), str(self.refill_per_ms), str(self.cost), str(self.ttl_ms)
            )
            return int(allowed), int(retry_ms), int(remaining)
        except ResponseError as e:
            # If Redis was restarted, it will forget the script
            if "NOSCRIPT" in str(e):
                await self.call(key)
            raise
        except Exception as e:
            # NOTE: Policy decision: fail-open (allow) or fail-close (deny)
            if self.fail_open:
                return 1, 0, float(self.capacity)
            return 0, 1000, 0.0 
