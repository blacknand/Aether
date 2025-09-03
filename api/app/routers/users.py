# api/app/routers/users.py

from fastapi import APIRouter, Depends
from typing import Annotated
from app.core.dependencies import get_current_active_user
from app.models.user import UserInDB

router = APIRouter(prefix="/users", tags=["users"])

@router.get("/me")
async def me(current: Annotated[UserInDB, Depends(get_current_active_user)]):
    return {"username": current.username, "email": current.email}