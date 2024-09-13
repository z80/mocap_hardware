
import json
import asyncio
import websockets


async def websocket_handler( websocket, path, clients_list, shared_data ):
    clients_list.append( websocket )
    try:
        while True:
            message = await websocket.recv()
            print(f"Received WebSocket message: {message}")
            # Process the message and send a response
            response = f"Processed message: {message}"
            await websocket.send(response)
            # Optionally, send the latest Bluetooth data
            if 'latest_bluetooth_data' in shared_data:
                await websocket.send(f"Latest Bluetooth data: {shared_data['latest_bluetooth_data']}")
    except websockets.ConnectionClosed:
        print( "Websockets client disconnected" )

    finally:
        clients_list.remove( websocket )




async def websocket_service( clients_list, shared_data ):
    async with websockets.serve( lambda ws, path: websocket_handler(ws, path, clients_list, shared_data), "localhost", 8765 ):
        await asyncio.Future()  # Run forever











