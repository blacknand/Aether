# api/app/core/dependencies.py

from typing import Annotated
from fastapi import Depends, HTTPException, status, Security
from fastapi.security import OAuth2PasswordBearer, SecurityScopes
from app.core.security import decode_access_token
from app.models.user import UserInDB, User
from app.models.auth import TokenData
from app.services.user_repo import get_user

oauth2_scheme = OAuth2PasswordBearer(
    tokenUrl="/auth/token",
    scopes={
        "me": "Read information about the current user.",
        "orders": "Read information about the orders available",
    },
)

async def get_current_user(
    token: Annotated[str, Depends(oauth2_scheme)],
    security_scopes: SecurityScopes
) -> UserInDB:
    if security_scopes.scopes: authenticate_value = f"Bearer scope={security_scopes.scope_str}"
    else: authenticate_value = "Bearer"
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Could not validate credentials",
        headers={"WWW-Authenticate": authenticate_value}
    )
    try:
        payload = decode_access_token(token)
        sub = payload.get("sub")
        raw_scopes = payload.get("scopes") or payload.get("scope") or []
        if isinstance(raw_scopes, str): scopes_list = raw_scopes.split()
        else: scopes_list = list(raw_scopes)
        token_data = TokenData(sub=sub, scopes=scopes_list)
    except Exception:
        raise credentials_exception

    user = get_user(token_data.sub) if token_data.sub else None
    if not user: raise credentials_exception

    for scope in security_scopes.scopes:
        if scope not in token_data.scopes:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Not enough permissions",
                headers={"WWW-Authenticate", authenticate_value}
            )

    return user

async def get_current_active_user(current_user: Annotated[User, Security(get_current_user, scopes=["me"])]):
    if current_user.disabled: raise HTTPException(status_code=400, detail="Inactive User")
    return current_user