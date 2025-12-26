import asyncio
import json

import websockets
from spotify import get_now_playing

clients = set()


async def handler(ws):
    clients.add(ws)
    try:
        async for _ in ws:
            pass
    finally:
        clients.remove(ws)


async def main():
    server = await websockets.serve(handler, "0.0.0.0", 8765)

    last_payload = None

    while True:
        data = get_now_playing()

        if data:
            payload = json.dumps({
                "type": "now_playing",
                "track": data["track"],
                "artist": data["artist"],
                "progress": data["progress_ms"],
                "duration": data["duration_ms"],
                "playing": data["is_playing"]
            })

            if payload != last_payload:
                await asyncio.gather(
                    *[ws.send(payload) for ws in clients],
                    return_exceptions=True
                )
                last_payload = payload

        await asyncio.sleep(1)

asyncio.run(main())
