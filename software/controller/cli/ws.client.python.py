#!/usr/bin/env python
#
# Python CLI ws console, useful for debugging
#
# pyright: reportMissingImports=false
import sys
try:
    import asyncio
    import websockets
except Exception as E:
    print(f"\nERROR: Import errors detected\nERROR: {str(E)}\n")
    sys.exit(1)

async def send_command():
    uri = "ws://192.168.0.230/ws"
    print(f"# URL {uri}, opening connection...")
    try:
        async with websockets.connect(uri) as websocket:
            print("# Remote host connected, now type your command or 'exit' to quit\n")
            while True:
                command = input("> ")
                if command == 'exit':
                    print("  Closing application\n")
                    break
                await websocket.send(command)
                response = await websocket.recv()
                print(f"< {response}")
    except KeyboardInterrupt:
        print("\n")

print("\n# Program started")
asyncio.get_event_loop().run_until_complete(send_command())
