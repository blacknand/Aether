from pydantic import BaseModel
import os

class Settings(BaseModel):
    SECRET_KEY: str = "e599a14254ae45a2eb830c6a64e217f1c9ebf83c8a8832c5f20537f0e102bcfd"         # Generated w openssl rand -hex 32
    ALGORITHM: str = "HS256"        # Maybe RS256 with keypair -- for microservices
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 30
    REDIS_URL="redis://localhost:6379/0"
    RL_CAPACITY=10
    RL_REFILL_PER_SPEC=5
    RL_TTL_MS=60000

settings = Settings()