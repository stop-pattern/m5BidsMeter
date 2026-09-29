"""Reply to Core2 BIDS polling over a real serial port.

Requires pyserial. The demo cycles through running, rolling warning, and
recovery states so the on-device display can be inspected without a game.
"""

import argparse
import collections
import sys
import time

import serial


CORE_KEYS = ("TRIE1", "TRIE3", "TRIE4", "TRIH0", "TRIH1")


def value_for(command: str, elapsed: float, version: int):
    if command == "TRV202":
        return version
    phase = int(elapsed // 5) % 3
    readings = (
        {"TRIE1": 64, "TRIE3": 320, "TRIE4": 860, "TRIH0": 6, "TRIH1": -1},
        {"TRIE1": 0, "TRIE3": 150, "TRIE4": 820, "TRIH0": 9, "TRIH1": 0},
        {"TRIE1": 25, "TRIE3": 230, "TRIE4": 900, "TRIH0": 0, "TRIH1": 0},
    )
    if command in CORE_KEYS:
        return readings[phase][command]
    if not command.startswith("TRIP") or not command[4:].isdigit():
        return None
    index = int(command[4:])
    if index > 255:
        return None
    lamps = {2: 1, 6: 1, 19: 1, 23: 1, 73: 1, 92: 1, 155: 1,
             110: 1, 125: 1}
    if phase == 1:
        lamps.update({22: 1, 101: 1, 102: 1})
    return lamps.get(index, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Core2 serial port, e.g. COM<number>")
    parser.add_argument("--version", type=int, choices=(100, 202), default=202)
    parser.add_argument("--duration", type=float, default=16,
                        help="seconds to exchange messages")
    args = parser.parse_args()
    counts = collections.Counter()
    started = time.monotonic()
    with serial.Serial(args.port, 115200, timeout=0.25, write_timeout=1) as port:
        while time.monotonic() - started < args.duration:
            request = port.readline().decode("ascii", errors="ignore").strip()
            if not request:
                continue
            response = value_for(request, time.monotonic() - started, args.version)
            if response is None:
                continue
            port.write(f"{request}X{response}\n".encode("ascii"))
            counts[request] += 1
    print("BIDS mock requests:", ", ".join(f"{key}={counts[key]}" for key in CORE_KEYS))
    print("Version requests:", counts["TRV202"], "Panel requests:",
          sum(n for key, n in counts.items() if key.startswith("TRIP")))
    missing = [key for key in CORE_KEYS if counts[key] == 0]
    if missing:
        print("Missing core requests:", ", ".join(missing), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
