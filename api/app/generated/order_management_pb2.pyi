from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from collections.abc import Iterable as _Iterable, Mapping as _Mapping
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class OrderSide(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    SIDE_UNKNOWN: _ClassVar[OrderSide]
    BID: _ClassVar[OrderSide]
    ASK: _ClassVar[OrderSide]

class OrderType(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    TYPE_UNKNOWN: _ClassVar[OrderType]
    MARKET: _ClassVar[OrderType]
    LIMIT: _ClassVar[OrderType]
SIDE_UNKNOWN: OrderSide
BID: OrderSide
ASK: OrderSide
TYPE_UNKNOWN: OrderType
MARKET: OrderType
LIMIT: OrderType

class OrderRequest(_message.Message):
    __slots__ = ("security_id", "side", "type", "quantity", "price")
    SECURITY_ID_FIELD_NUMBER: _ClassVar[int]
    SIDE_FIELD_NUMBER: _ClassVar[int]
    TYPE_FIELD_NUMBER: _ClassVar[int]
    QUANTITY_FIELD_NUMBER: _ClassVar[int]
    PRICE_FIELD_NUMBER: _ClassVar[int]
    security_id: int
    side: OrderSide
    type: OrderType
    quantity: int
    price: int
    def __init__(self, security_id: _Optional[int] = ..., side: _Optional[_Union[OrderSide, str]] = ..., type: _Optional[_Union[OrderType, str]] = ..., quantity: _Optional[int] = ..., price: _Optional[int] = ...) -> None: ...

class OrderConfirmation(_message.Message):
    __slots__ = ("order_id", "accepted", "reason", "trades")
    ORDER_ID_FIELD_NUMBER: _ClassVar[int]
    ACCEPTED_FIELD_NUMBER: _ClassVar[int]
    REASON_FIELD_NUMBER: _ClassVar[int]
    TRADES_FIELD_NUMBER: _ClassVar[int]
    order_id: int
    accepted: bool
    reason: str
    trades: _containers.RepeatedCompositeFieldContainer[Trade]
    def __init__(self, order_id: _Optional[int] = ..., accepted: bool = ..., reason: _Optional[str] = ..., trades: _Optional[_Iterable[_Union[Trade, _Mapping]]] = ...) -> None: ...

class StreamRequest(_message.Message):
    __slots__ = ("security_id",)
    SECURITY_ID_FIELD_NUMBER: _ClassVar[int]
    security_id: int
    def __init__(self, security_id: _Optional[int] = ...) -> None: ...

class Trade(_message.Message):
    __slots__ = ("trade_id", "security_id", "price", "quantity", "timestamp_ns")
    TRADE_ID_FIELD_NUMBER: _ClassVar[int]
    SECURITY_ID_FIELD_NUMBER: _ClassVar[int]
    PRICE_FIELD_NUMBER: _ClassVar[int]
    QUANTITY_FIELD_NUMBER: _ClassVar[int]
    TIMESTAMP_NS_FIELD_NUMBER: _ClassVar[int]
    trade_id: int
    security_id: int
    price: int
    quantity: int
    timestamp_ns: int
    def __init__(self, trade_id: _Optional[int] = ..., security_id: _Optional[int] = ..., price: _Optional[int] = ..., quantity: _Optional[int] = ..., timestamp_ns: _Optional[int] = ...) -> None: ...

class OrderBookLevel(_message.Message):
    __slots__ = ("price", "aggregate_quantity")
    PRICE_FIELD_NUMBER: _ClassVar[int]
    AGGREGATE_QUANTITY_FIELD_NUMBER: _ClassVar[int]
    price: int
    aggregate_quantity: int
    def __init__(self, price: _Optional[int] = ..., aggregate_quantity: _Optional[int] = ...) -> None: ...

class OrderBookSnapshot(_message.Message):
    __slots__ = ("security_id", "bids", "asks", "timestamp_ns")
    SECURITY_ID_FIELD_NUMBER: _ClassVar[int]
    BIDS_FIELD_NUMBER: _ClassVar[int]
    ASKS_FIELD_NUMBER: _ClassVar[int]
    TIMESTAMP_NS_FIELD_NUMBER: _ClassVar[int]
    security_id: int
    bids: _containers.RepeatedCompositeFieldContainer[OrderBookLevel]
    asks: _containers.RepeatedCompositeFieldContainer[OrderBookLevel]
    timestamp_ns: int
    def __init__(self, security_id: _Optional[int] = ..., bids: _Optional[_Iterable[_Union[OrderBookLevel, _Mapping]]] = ..., asks: _Optional[_Iterable[_Union[OrderBookLevel, _Mapping]]] = ..., timestamp_ns: _Optional[int] = ...) -> None: ...
