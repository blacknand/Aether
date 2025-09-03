# api/app/services/auth_service.py

from datetime import timedelta
from app.core.security import verify_password, create_access_token, hash_password
from app.models.user import UserInDB, RegisterRequest
from app.models.auth import Token
from app.services.user_repo import get_user, create_user

class AuthService:
    def authenticate(self, username: str, password: str) -> UserInDB | None:
        user = get_user(username)
        if not user: return None
        if not verify_password(password, user.hashed_password): return None
        return user
    
    def issue_token(self, data: dict, expire_delta: timedelta = timedelta(minutes=30)) -> Token:
        jwt_str = create_access_token(data=data, expires_delta=expire_delta)
        return Token(access_token=jwt_str)
    
    def register(self, req: RegisterRequest) -> UserInDB:
        u = UserInDB(username=req.username,
                     email=req.email,
                     hashed_password=hash_password(req.password))
        return create_user(u)