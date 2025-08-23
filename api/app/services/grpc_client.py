import logging
import grpc
from routers.orders import OrderSide, OrderType, SubmitOrderRequest
from generated.order_management_pb2_grpc import MatchingEngineStub
from generated.market_data_pb2_grpc import StreamOrderBookStateStub


class GrpcClient:
    def __cls__(cls, target: str):
        pass

    def __init__(self, target: str):
        self._target = target
        self._channel = None
        self.matching_engine_stub = None
        self.stream_order_book_state_stub = None
        logging.basicConfig(level=logging.INFO)

    async def connect(self) -> None:
        self._channel = grpc.insecure_channel("localhost:50051")
        self.matching_engine_stub = MatchingEngineStub(self._channel)
        self.stream_order_book_state_stub = StreamOrderBookStateStub(self._channel)
        logging.info("[py] gRPC client connected to localhost:50051.")

    async def disconnect(self) -> None:
        if self._channel is not None:
            self._channel.close()
            self._channel = None
            logging.info("[py] gRPC client disconnected.")