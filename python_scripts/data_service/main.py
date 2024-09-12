
import asyncio
from uart_service       import uart_service
from processing_service import processing_service
from websocket_service  import websocket_service


async def main():
    loop = asyncio.get_running_loop()
    queue = asyncio.Queue()
    shared_data = {}  # Shared dictionary
    await asyncio.gather(
        uart_service(loop, queue),
        processing_service(queue, shared_data), 
        websocket_service(shared_data)
    )

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        # task is cancelled on disconnect, so we ignore this error
        pass


