# api/app/models/user.py
from pydantic import BaseModel, EmailStr

class User(BaseModel):
    username: str
    email: EmailStr
    disabled: bool = False

class UserInDB(User):
    hashed_password: str
    # TODO: add others later

class LoginRequest(BaseModel):
    username: str
    password: str

class RegisterRequest(BaseModel):
    username: str
    email: EmailStr
    password: str