#!/bin/bash
# Share-Y Launcher

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="$SCRIPT_DIR/venv"

# Check if virtual environment exists
if [ ! -d "$VENV_DIR" ]; then
    echo "Creating virtual environment..."
    python3 -m venv "$VENV_DIR"
    "$VENV_DIR/bin/pip" install PyQt6 pynput Pillow
fi

# Run the application
exec "$VENV_DIR/bin/python" "$SCRIPT_DIR/main.py" "$@"
