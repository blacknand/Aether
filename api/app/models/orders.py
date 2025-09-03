from pydantic import BaseModel, model_validator, Field
from app.generated.order_management_pb2 import OrderSide, OrderType
from typing_extensions import Self
from enum import Enum
from typing import Annotated, Optional


class OrderSide(str, Enum):
    BID = "BID"
    ASK = "ASK"


class OrderType(str, Enum):
    LIMIT = "LIMIT"
    MARKET = "MARKET"


class SubmitOrderRequest(BaseModel):
    security_id: Annotated[int, Field(strict=True, gt=0)]
    side: OrderSide 
    type: OrderType 
    quantity: Annotated[int, Field(strict=True, gt=0)] 
    price: Optional[Annotated[int, Field(strict=True, gt=0)]] = None

    @model_validator(mode="after")
    def check_order_request(self) -> Self:
        if self.type == OrderType.MARKET and self.price is not None:
            raise ValueError("Submitted a market order but, provided a price.")
        elif self.type == OrderType.LIMIT and self.price is None:
            raise ValueError("Submitted a limit order, but did not provide a price.")
        return self

