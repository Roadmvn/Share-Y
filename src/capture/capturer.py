"""
Screen Capture Module

Uses system screenshot tools (gnome-screenshot, flameshot) to capture screens.
This approach doesn't interfere with the default system screenshot tool.
"""

import os
import subprocess
import tempfile
import time
import glob
from pathlib import Path
from typing import Optional, List

from PyQt6.QtGui import QPixmap

from ..core.screenshot import Screenshot, CaptureType


class Capturer:
    """
    Screen capture engine using system tools
    
    Automatically detects available capture tools and uses the best one.
    Saves to temp file then loads into memory.
    """
    
    def __init__(self):
        self._tool = self._detect_tool()
        self._temp_dir = tempfile.gettempdir()
    
    def _detect_tool(self) -> Optional[str]:
        """Detect available screenshot tool"""
        # Prefer gnome-screenshot for better control
        tools = ["gnome-screenshot", "scrot", "maim", "flameshot"]
        
        for tool in tools:
            try:
                result = subprocess.run(
                    ["which", tool],
                    capture_output=True,
                    timeout=2
                )
                if result.returncode == 0:
                    print(f"[Capturer] Using {tool} for screenshots")
                    return tool
            except Exception:
                continue
        
        print("[Capturer] Warning: No screenshot tool found!")
        return None
    
    def is_ready(self) -> bool:
        """Check if capture is available"""
        return self._tool is not None
    
    def _generate_temp_path(self) -> str:
        """Generate a unique temp file path"""
        return os.path.join(
            self._temp_dir,
            f"sharey_capture_{int(time.time() * 1000)}.png"
        )
    
    def _find_latest_screenshot(self, directories: List[str], before_time: float) -> Optional[str]:
        """Find the latest screenshot file created after before_time"""
        latest_file = None
        latest_mtime = before_time
        
        for directory in directories:
            if not os.path.exists(directory):
                continue
            
            for pattern in ["*.png", "*.PNG"]:
                for filepath in glob.glob(os.path.join(directory, pattern)):
                    try:
                        mtime = os.path.getmtime(filepath)
                        if mtime > latest_mtime:
                            latest_mtime = mtime
                            latest_file = filepath
                    except Exception:
                        continue
        
        return latest_file
    
    def _capture_with_gnome_screenshot(self, region: bool = False) -> Optional[str]:
        """Capture using gnome-screenshot"""
        temp_path = self._generate_temp_path()
        
        try:
            if region:
                cmd = ["gnome-screenshot", "-a", "-f", temp_path]
            else:
                cmd = ["gnome-screenshot", "-f", temp_path]
            
            print(f"[Capturer] Running: {' '.join(cmd)}")
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                timeout=120 if region else 10
            )
            
            if result.returncode == 0 and os.path.exists(temp_path):
                print(f"[Capturer] Screenshot saved to: {temp_path}")
                return temp_path
            else:
                print(f"[Capturer] Capture failed or cancelled")
                return None
                
        except subprocess.TimeoutExpired:
            print("[Capturer] Capture timed out")
            return None
        except Exception as e:
            print(f"[Capturer] Error: {e}")
            return None
    
    def _capture_with_flameshot(self, region: bool = False) -> Optional[str]:
        """Capture using flameshot - handles its unique save behavior"""
        temp_path = self._generate_temp_path()
        
        # Get possible screenshot directories
        home = os.path.expanduser("~")
        screenshot_dirs = [
            os.path.join(home, "Pictures", "Screenshots"),
            os.path.join(home, "Pictures"),
            os.path.join(home, "Images", "Screenshots"),
            os.path.join(home, "Images"),
            os.path.join(home, "Images", "Captures d'écran"),
            self._temp_dir,
        ]
        
        before_time = time.time()
        
        try:
            if region:
                # Use flameshot gui with raw output to stdout
                cmd = ["flameshot", "gui", "--raw"]
                print(f"[Capturer] Running: {' '.join(cmd)}")
                
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    timeout=120
                )
                
                if result.returncode == 0 and result.stdout:
                    # Save raw data to temp file
                    with open(temp_path, 'wb') as f:
                        f.write(result.stdout)
                    print(f"[Capturer] Screenshot saved to: {temp_path}")
                    return temp_path
            else:
                # Fullscreen with path
                cmd = ["flameshot", "full", "--raw"]
                print(f"[Capturer] Running: {' '.join(cmd)}")
                
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    timeout=10
                )
                
                if result.returncode == 0 and result.stdout:
                    with open(temp_path, 'wb') as f:
                        f.write(result.stdout)
                    print(f"[Capturer] Screenshot saved to: {temp_path}")
                    return temp_path
            
            print("[Capturer] Capture failed or cancelled")
            return None
                
        except subprocess.TimeoutExpired:
            print("[Capturer] Capture timed out")
            return None
        except Exception as e:
            print(f"[Capturer] Error: {e}")
            return None
    
    def _capture_with_scrot(self, region: bool = False) -> Optional[str]:
        """Capture using scrot"""
        temp_path = self._generate_temp_path()
        
        try:
            if region:
                cmd = ["scrot", "-s", temp_path]
            else:
                cmd = ["scrot", temp_path]
            
            print(f"[Capturer] Running: {' '.join(cmd)}")
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                timeout=120 if region else 10
            )
            
            if result.returncode == 0 and os.path.exists(temp_path):
                return temp_path
            return None
                
        except Exception as e:
            print(f"[Capturer] Error: {e}")
            return None
    
    def _capture_with_maim(self, region: bool = False) -> Optional[str]:
        """Capture using maim"""
        temp_path = self._generate_temp_path()
        
        try:
            if region:
                cmd = ["maim", "-s", temp_path]
            else:
                cmd = ["maim", temp_path]
            
            print(f"[Capturer] Running: {' '.join(cmd)}")
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                timeout=120 if region else 10
            )
            
            if result.returncode == 0 and os.path.exists(temp_path):
                return temp_path
            return None
                
        except Exception as e:
            print(f"[Capturer] Error: {e}")
            return None
    
    def _load_and_cleanup(self, path: str) -> Optional[QPixmap]:
        """Load image from path and delete temp file"""
        if not path or not os.path.exists(path):
            return None
        
        pixmap = QPixmap(path)
        
        # Only remove if it's our temp file
        if path.startswith(self._temp_dir):
            try:
                os.remove(path)
            except Exception:
                pass
        
        if pixmap.isNull():
            print(f"[Capturer] Failed to load image from {path}")
            return None
        
        print(f"[Capturer] Loaded image: {pixmap.width()}x{pixmap.height()}")
        return pixmap
    
    def _do_capture(self, region: bool = False) -> Optional[str]:
        """Perform capture based on detected tool"""
        if self._tool == "gnome-screenshot":
            return self._capture_with_gnome_screenshot(region)
        elif self._tool == "flameshot":
            return self._capture_with_flameshot(region)
        elif self._tool == "scrot":
            return self._capture_with_scrot(region)
        elif self._tool == "maim":
            return self._capture_with_maim(region)
        return None
    
    def capture_fullscreen(self) -> Optional[Screenshot]:
        """Capture the entire screen"""
        if not self.is_ready():
            return None
        
        path = self._do_capture(region=False)
        if not path:
            return None
        
        pixmap = self._load_and_cleanup(path)
        if not pixmap:
            return None
        
        screenshot = Screenshot.create(pixmap, CaptureType.FULLSCREEN)
        print(f"[Capturer] Created screenshot with ID: {screenshot.id}")
        return screenshot
    
    def capture_region(self) -> Optional[Screenshot]:
        """Capture a selected region (interactive)"""
        if not self.is_ready():
            return None
        
        path = self._do_capture(region=True)
        if not path:
            return None
        
        pixmap = self._load_and_cleanup(path)
        if not pixmap:
            return None
        
        screenshot = Screenshot.create(pixmap, CaptureType.REGION)
        print(f"[Capturer] Created screenshot with ID: {screenshot.id}")
        return screenshot
    
    def capture_window(self) -> Optional[Screenshot]:
        """Capture a specific window (interactive)"""
        # Fall back to region capture for window selection
        return self.capture_region()
    
    def capture_active_window(self) -> Optional[Screenshot]:
        """Capture the currently active window"""
        # Fall back to fullscreen for now
        return self.capture_fullscreen()

