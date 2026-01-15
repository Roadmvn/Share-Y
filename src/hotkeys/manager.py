"""
Global Hotkey Manager

Uses pynput to register global keyboard shortcuts.
Designed to use different shortcuts than system defaults to avoid conflicts.
"""

import threading
from typing import Callable, Dict, Optional
from pynput import keyboard


class HotkeyManager:
    """
    Global hotkey manager using pynput
    
    Registers keyboard shortcuts that work globally.
    Uses Ctrl+Alt combinations to avoid conflicts with system shortcuts.
    """
    
    def __init__(self):
        self._hotkeys: Dict[str, Callable] = {}
        self._listener: Optional[keyboard.GlobalHotKeys] = None
        self._running = False
    
    def _parse_hotkey(self, shortcut: str) -> str:
        """Convert shortcut string to pynput format"""
        # Convert format: "Ctrl+Alt+P" -> "<ctrl>+<alt>+p"
        parts = shortcut.lower().split("+")
        result = []
        
        for part in parts:
            part = part.strip()
            if part in ("ctrl", "control"):
                result.append("<ctrl>")
            elif part in ("alt",):
                result.append("<alt>")
            elif part in ("shift",):
                result.append("<shift>")
            elif part in ("super", "win", "meta"):
                result.append("<cmd>")
            else:
                result.append(part)
        
        return "+".join(result)
    
    def register_hotkey(self, shortcut: str, callback: Callable):
        """
        Register a global hotkey
        
        Args:
            shortcut: Human-readable shortcut like "Ctrl+Alt+P"
            callback: Function to call when hotkey is pressed
        """
        pynput_key = self._parse_hotkey(shortcut)
        self._hotkeys[pynput_key] = callback
        print(f"[HotkeyManager] Registered: {shortcut} -> {pynput_key}")
    
    def unregister_hotkey(self, shortcut: str):
        """Unregister a hotkey"""
        pynput_key = self._parse_hotkey(shortcut)
        if pynput_key in self._hotkeys:
            del self._hotkeys[pynput_key]
    
    def start(self):
        """Start listening for hotkeys"""
        if self._running:
            return
        
        if not self._hotkeys:
            print("[HotkeyManager] No hotkeys registered")
            return
        
        self._listener = keyboard.GlobalHotKeys(self._hotkeys)
        self._listener.start()
        self._running = True
        print(f"[HotkeyManager] Started with {len(self._hotkeys)} hotkeys")
    
    def stop(self):
        """Stop listening for hotkeys"""
        if self._listener:
            self._listener.stop()
            self._listener = None
        self._running = False
        print("[HotkeyManager] Stopped")
    
    def is_running(self) -> bool:
        """Check if hotkey listener is active"""
        return self._running
