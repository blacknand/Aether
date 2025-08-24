import logging
import grpc
from app.generated.order_management_pb2_grpc import MatchingEngineStub
from app.generated.market_data_pb2_grpc import StreamOrderBookStateStub


class GrpcClient:
    def __init__(self, target: str):
        self._target = target
        self._channel = None
        self._connected = False
        self.matching_engine_stub = None
        self.stream_order_book_state_stub = None

    async def connect(self) -> None:
        # self._channel = grpc.insecure_channel("localhost:50051")
        self.matching_engine_stub = MatchingEngineStub(self._channel)
        self.stream_order_book_state_stub = StreamOrderBookStateStub(self._channel)
        self._connected = True

    async def disconnect(self) -> None:
        if self._channel is not None:
            self._channel.close()
            self._channel = None
            self._connected = False

    @property
    def is_connected(self) -> bool:
        return self._connected

_client_singleton: GrpcClient | None = None

def get_client(target: str = "localhost:50051") -> GrpcClient:
    global _client_singleton
    if _client_singleton is None:
        _client_singleton = GrpcClient(target)
    return _client_singleton