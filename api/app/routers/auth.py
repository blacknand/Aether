# app/routes/auth.py

from datetime import timedelta
from fastapi import APIRouter, HTTPException, status, Depends
from fastapi.security import OAuth2PasswordRequestForm
from typing import Dict, Annotated
from app.models.user import RegisterRequest
from app.models.auth import Token
from app.core.config import settings
from app.services.auth_service import AuthService

router = APIRouter(prefix="/auth", tags=["auth"])
svc = AuthService()

@router.post("/token", response_model=Token)
async def login_for_access_token(form_data: Annotated[OAuth2PasswordRequestForm, Depends()]) -> Token:
    user = svc.authenticate(form_data.username, form_data.password)
    if not user: 
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED, 
            detail="Bad credentials",
            headers={"WWW-Authenticate": "Bearer"}
        )
    return svc.issue_token(
        data={"sub": user.username, "scopes": form_data.scopes},
        expire_delta=timedelta(minutes=settings.ACCESS_TOKEN_EXPIRE_MINUTES)
    )

@router.post("/register")
def register(req: RegisterRequest) -> Dict:
    user = svc.register(req)
    return {"username": user.username, "email": user.email}