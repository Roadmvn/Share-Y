"""
Screenshot data structure

Holds the image data in memory along with metadata.
Uses QPixmap for efficient rendering.
"""

from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum
from typing import Optional
from uuid import UUID, uuid4

from PyQt6.QtGui import QPixmap
from PyQt6.QtCore import Qt, QSize


class CaptureType(Enum):
    """Capture type enumeration"""
    FULLSCREEN = "fullscreen"
    REGION = "region"
    WINDOW = "window"
    ACTIVE_WINDOW = "active_window"


@dataclass
class Screenshot:
    """
    Screenshot container
    
    Stores image data in RAM with associated metadata.
    """
    id: UUID = field(default_factory=uuid4)
    pixmap: QPixmap = field(default_factory=QPixmap)
    timestamp: datetime = field(default_factory=datetime.now)
    capture_type: CaptureType = CaptureType.FULLSCREEN
    window_title: str = ""
    original_size: QSize = field(default_factory=lambda: QSize(0, 0))
    has_annotations: bool = False
    
    @classmethod
    def create(cls, pixmap: QPixmap, capture_type: CaptureType, 
               title: str = "") -> "Screenshot":
        """Create a new screenshot from a pixmap"""
        return cls(
            id=uuid4(),
            pixmap=pixmap,
            timestamp=datetime.now(),
            capture_type=capture_type,
            window_title=title,
            original_size=pixmap.size()
        )
    
    def thumbnail(self, max_size: int = 200) -> QPixmap:
        """Generate a thumbnail for display"""
        return self.pixmap.scaled(
            max_size, max_size,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        )
    
    def size_string(self) -> str:
        """Get human-readable size"""
        return f"{self.original_size.width()} x {self.original_size.height()}"
    
    def memory_usage(self) -> int:
        """Estimate memory usage in bytes"""
        return (self.pixmap.width() * self.pixmap.height() * 
                (self.pixmap.depth() // 8))
    
    def is_valid(self) -> bool:
        """Check if screenshot has valid pixmap"""
        return not self.pixmap.isNull()
