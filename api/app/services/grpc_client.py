import asyncio
from grpc import aio
from app.generated.order_management_pb2_grpc import MatchingEngineStub
from app.generated.market_data_pb2_grpc import StreamOrderBookStateStub
from app.generated.order_management_pb2 import OrderRequest, OrderType, OrderSide

class GrpcClient:
    def __init__(self, target: str):
        self._target = target
        self._channel = None
        self._connected = False
        self.matching_engine_stub = None
        self.stream_order_book_state_stub = None

    async def connect(self) -> None:
        if self._connected: return
        self._channel = aio.insecure_channel(self._target)
        await self._channel.channel_ready()
        self.matching_engine_stub = MatchingEngineStub(self._channel)
        self.stream_order_book_state_stub = StreamOrderBookStateStub(self._channel)
        self._connected = True

    async def close(self) -> None:
        if self._channel is not None:
            await self._channel.close()
            self._channel = None
            self._connected = False

    async def submitOrder(self, orderRequest):        # NOTE: Do not know what it would return
        if not self._connected: return "[PyAPI] Not connected to Aether gRPC server"
        return await self.matching_engine_stub.SubmitOrder(orderRequest)

    @property
    def is_connected(self) -> bool:
        return self._connected

_client_singleton: GrpcClient | None = None

def get_client(target: str = "0.0.0.0:50051") -> GrpcClient:
    global _client_singleton
    if _client_singleton is None:
        _client_singleton = GrpcClient(target)
    return _client_singleton

client = get_client()

async def main():
    await client.connect()
    request = OrderRequest()
    request.security_id = 1001
    request.side = OrderSide.BID
    request.type = OrderType.LIMIT 
    request.quantity = 10
    request.price = 100
    result = await client.submitOrder(request)
    print(result)
    print(client.is_connected)
    await client.close()

asyncio.run(main())