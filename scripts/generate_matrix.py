import json

# === CONFIGURATION ===
# ZMK renamed the Nice!Nano v2 board from "nice_nano_v2" to "nice_nano" in newer trees.
board = "nice_nano"

groups = []
groups.append({
    "keymap": "qwerty",
    "format": "dongle",
    "name": "qwerty-dongle",
    "board": board,
})

groups.append({
    "keymap": "default",
    "format": "reset",
    "name": "reset-nanov2",
    "board": board,
})

# Dump matrix as compact JSON (GitHub expects it this way)
print(json.dumps(groups))
