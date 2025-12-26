import os

import spotipy
from dotenv import load_dotenv
from spotipy.oauth2 import SpotifyOAuth

load_dotenv()

sp = spotipy.Spotify(
    auth_manager=SpotifyOAuth(
        scope="user-read-playback-state",
        client_id=os.getenv("SPOTIPY_CLIENT_ID"),
        client_secret=os.getenv("SPOTIPY_CLIENT_SECRET"),
        redirect_uri=os.getenv("SPOTIPY_REDIRECT_URI")
    )
)


def get_now_playing():
    current = sp.current_playback()

    if not current or not current.get("item"):
        return None

    item = current["item"]

    return {
        "is_playing": current["is_playing"],
        "track": item["name"],
        "artist": ", ".join(a["name"] for a in item["artists"]),
        "progress_ms": current["progress_ms"],
        "duration_ms": item["duration_ms"]
    }
