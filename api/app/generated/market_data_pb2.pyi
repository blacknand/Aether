import order_management_pb2 as _order_management_pb2
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class DeltaAction(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    ADD: _ClassVar[DeltaAction]
    UPDATE: _ClassVar[DeltaAction]
    DELETE: _ClassVar[DeltaAction]
ADD: DeltaAction
UPDATE: DeltaAction
DELETE: DeltaAction

class OrderDelta(_message.Message):
    __slots__ = ("action", "side", "price", "quantity")
    ACTION_FIELD_NUMBER: _ClassVar[int]
    SIDE_FIELD_NUMBER: _ClassVar[int]
    PRICE_FIELD_NUMBER: _ClassVar[int]
    QUANTITY_FIELD_NUMBER: _ClassVar[int]
    action: DeltaAction
    side: _order_management_pb2.OrderSide
    price: int
    quantity: int
    def __init__(self, action: _Optional[_Union[DeltaAction, str]] = ..., side: _Optional[_Union[_order_management_pb2.OrderSide, str]] = ..., price: _Optional[int] = ..., quantity: _Optional[int] = ...) -> None: ...
