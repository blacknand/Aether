import logging
# Silence AttributeError: module 'bcrypt' has no attribute '__about__' error
logging.getLogger("passlib").setLevel(logging.ERROR)
logging.getLogger("passlib.hanlders.bcrypt").setLevel(logging.ERROR)

from fastapi import FastAPI
from app.routers import auth, users, orders

app = FastAPI(title="Aether Gateway")

app.include_router(auth.router)
app.include_router(users.router)
app.include_router(orders.router)