
import json
import asyncio
import websockets


async def websocket_handler( websocket, path, shared_data ):
    message = await websocket.recv()
    print(f"Received WebSocket message: {message}")
    # Process the message and send a response
    response = f"Processed message: {message}"
    await websocket.send(response)
    # Optionally, send the latest Bluetooth data
    if 'latest_bluetooth_data' in shared_data:
        await websocket.send(f"Latest Bluetooth data: {shared_data['latest_bluetooth_data']}")




async def websocket_service( shared_data ):
    async with websockets.serve( lambda ws, path: websocket_handler(ws, path, shared_data), "localhost", 8765 ):
        await asyncio.Future()  # Run forever











