"""
Screenshot Buffer

In-memory storage for screenshots with configurable memory limit.
Automatically evicts oldest screenshots when limit is reached.
"""

from typing import Dict, Optional, List
from uuid import UUID

from PyQt6.QtCore import QObject, pyqtSignal

from .screenshot import Screenshot


class ScreenshotBuffer(QObject):
    """
    In-memory screenshot storage
    
    Stores screenshots in RAM with a configurable memory limit.
    Emits signals when screenshots are added/removed.
    """
    
    # Signals
    screenshot_added = pyqtSignal(object)  # UUID
    screenshot_removed = pyqtSignal(object)  # UUID
    buffer_cleared = pyqtSignal()
    
    # Default memory limit: 500 MB
    DEFAULT_MEMORY_LIMIT = 500 * 1024 * 1024
    
    def __init__(self, memory_limit: int = DEFAULT_MEMORY_LIMIT):
        super().__init__()
        self._screenshots: Dict[UUID, Screenshot] = {}
        self._order: List[UUID] = []  # For FIFO eviction
        self._memory_limit = memory_limit
    
    @property
    def memory_limit(self) -> int:
        """Get memory limit in bytes"""
        return self._memory_limit
    
    @memory_limit.setter
    def memory_limit(self, value: int):
        """Set memory limit and evict if necessary"""
        self._memory_limit = value
        self._evict_if_needed()
    
    def add(self, screenshot: Screenshot) -> bool:
        """
        Add a screenshot to the buffer
        
        Returns True if added successfully, False if screenshot is invalid.
        """
        if not screenshot.is_valid():
            return False
        
        # Check if we need to evict old screenshots
        self._evict_if_needed(screenshot.memory_usage())
        
        self._screenshots[screenshot.id] = screenshot
        self._order.append(screenshot.id)
        
        self.screenshot_added.emit(screenshot.id)
        return True
    
    def remove(self, screenshot_id: UUID) -> bool:
        """Remove a screenshot by ID"""
        if screenshot_id not in self._screenshots:
            return False
        
        del self._screenshots[screenshot_id]
        self._order.remove(screenshot_id)
        
        self.screenshot_removed.emit(screenshot_id)
        return True
    
    def get(self, screenshot_id: UUID) -> Optional[Screenshot]:
        """Get a screenshot by ID"""
        return self._screenshots.get(screenshot_id)
    
    def clear(self):
        """Clear all screenshots from buffer"""
        self._screenshots.clear()
        self._order.clear()
        self.buffer_cleared.emit()
    
    def count(self) -> int:
        """Get number of screenshots in buffer"""
        return len(self._screenshots)
    
    def total_memory_usage(self) -> int:
        """Get total memory usage in bytes"""
        return sum(s.memory_usage() for s in self._screenshots.values())
    
    def all_screenshots(self) -> List[Screenshot]:
        """Get all screenshots in order (oldest first)"""
        return [self._screenshots[id] for id in self._order 
                if id in self._screenshots]
    
    def _evict_if_needed(self, additional_bytes: int = 0):
        """Evict oldest screenshots if memory limit would be exceeded"""
        target = self._memory_limit - additional_bytes
        
        while self._order and self.total_memory_usage() > target:
            oldest_id = self._order[0]
            self.remove(oldest_id)
    
    def memory_usage_string(self) -> str:
        """Get human-readable memory usage"""
        usage = self.total_memory_usage()
        
        if usage < 1024:
            return f"{usage} B"
        elif usage < 1024 * 1024:
            return f"{usage / 1024:.1f} KB"
        else:
            return f"{usage / (1024 * 1024):.1f} MB"
