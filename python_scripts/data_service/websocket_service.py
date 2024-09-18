
import json
import asyncio
import websockets


async def websocket_handler( websocket, path, clients_list ):
    clients_list.append( websocket )
    try:
        while True:
            message = await websocket.recv()
            print(f"Received WebSocket message: {message}")
    except websockets.ConnectionClosed:
        print( "Websockets client disconnected" )

    finally:
        clients_list.remove( websocket )


async def broadcast_data( clients_list, shared_data, semaphore ):
    while True:
        await semaphore.acquire()
        if clients_list:
            data_stri = json.dumps( shared_data )
            #await asyncio.wait( [ client.send(data_stri) for client in clients_list ] )



async def websocket_service( clients_list, shared_data, semaphore ):
    server = websockets.serve( lambda ws, path: websocket_handler(ws, path, clients_list), "localhost", 8765 )
    await asyncio.gather(
        server #, 
        #broadcast_data( clients_list, shared_data, semaphore )
        )











