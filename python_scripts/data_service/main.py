
import asyncio
from uart_service       import uart_service
from processing_service import processing_service
from websocket_service  import websocket_service


async def main():
    loop = asyncio.get_running_loop()
    queue = asyncio.Queue()
    clients_list = []
    shared_data = {}  # Shared dictionary
    semaphore = asyncio.Semaphore(0)

    await asyncio.gather(
        uart_service(loop, queue),
        processing_service(queue, clients_list, shared_data, semaphore), 
        websocket_service(clients_list, shared_data, semaphore)
    )

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        # task is cancelled on disconnect, so we ignore this error
        pass


