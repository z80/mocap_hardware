
from bleak import BleakScanner

async def run():
    print( "\n" * 10 )
    print("=" * 50)
    devices = await BleakScanner.discover()
    for device in devices:
        print(device)
        print(f"Found device: {device.name} ({device.address})")
        print( "metadata: ", device.metadata )
import asyncio
asyncio.run(run())


