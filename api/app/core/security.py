from datetime import datetime, timedelta, timezone
from jose import jwt 
from passlib.context import CryptContext
from app.core.config import settings

pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

def hash_password(password: str) -> str: return pwd_context.hash(password)

def verify_password(password: str, hashed: str) -> bool: return pwd_context.verify(password, hashed)

def create_access_token(data: dict, expires_delta: timedelta = timedelta(minutes=30)) -> str:
    to_encode = data.copy() if data else {}
    exp = datetime.now(tz=timezone.utc) + expires_delta
    to_encode.update({"exp": exp})
    return jwt.encode(to_encode, settings.SECRET_KEY, algorithm=settings.ALGORITHM)
    
def decode_access_token(token: str) -> dict: return jwt.decode(token, settings.SECRET_KEY, algorithms=[settings.ALGORITHM])