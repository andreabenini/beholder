import asyncio
import websockets

async def send_command():
    uri = "ws://<ESP32_IP_ADDRESS>/ws"  # Replace <ESP32_IP_ADDRESS> with the actual IP address of your ESP32

    async with websockets.connect(uri) as websocket:
        while True:
            command = input("Enter command (or 'exit' to quit): ")
            if command == 'exit':
                break
            await websocket.send(command)
            response = await websocket.recv()
            print("Response from server:", response)

asyncio.get_event_loop().run_until_complete(send_command())
