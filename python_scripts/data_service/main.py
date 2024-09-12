
import asyncio
from uart_service       import uart_service
from processing_service import processing_service
from websocker_service  import websocket_service


async def main():
    queue = asyncio.Queue()
    clients = set()
    shared_data = {}  # Shared dictionary
    await asyncio.gather(
        uart_service(queue, shared_data),
        processing_service(queue, clients, shared_data), 
        websocket_service(queue, clients, shared_data)
    )

if __name__ == "__main__":
    asyncio.run(main())



