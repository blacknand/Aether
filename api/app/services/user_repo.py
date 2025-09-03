# api/app/services/user_repo.py

from typing import Optional, Dict
from app.models.user import UserInDB
from app.core.security import hash_password

_FAKE_DB: Dict[str, UserInDB] = {
    "Joe": UserInDB(username="Joe",
                    email="kd63.7@lapd.gov.us",
                    hashed_password=hash_password("internlinked"))
}

def get_user(username: str) -> Optional[UserInDB]: return _FAKE_DB.get(username)

def create_user(u: UserInDB) -> UserInDB:
    if u.username in _FAKE_DB:
        raise ValueError("Username taken")
    _FAKE_DB[u.username] = u
    return u