"""
Thumbnail Widget

A clickable widget that displays a screenshot thumbnail.
Supports selection, double-click to copy, and context menu.
"""

from uuid import UUID

from PyQt6.QtWidgets import (QFrame, QVBoxLayout, QLabel, QMenu, 
                              QSizePolicy, QApplication)
from PyQt6.QtCore import Qt, pyqtSignal, QSize
from PyQt6.QtGui import QPixmap, QMouseEvent, QContextMenuEvent

from ..core.screenshot import Screenshot


class ThumbnailWidget(QFrame):
    """
    Clickable thumbnail widget for screenshot preview
    """
    
    # Signals
    clicked = pyqtSignal(object)  # UUID
    double_clicked = pyqtSignal(object)  # UUID
    copy_requested = pyqtSignal(object)  # UUID
    save_requested = pyqtSignal(object)  # UUID
    delete_requested = pyqtSignal(object)  # UUID
    
    THUMBNAIL_SIZE = 180
    
    def __init__(self, screenshot: Screenshot, parent=None):
        super().__init__(parent)
        
        self._screenshot = screenshot
        self._selected = False
        
        self._setup_ui()
        self._apply_style()
    
    def _setup_ui(self):
        """Set up the widget UI"""
        self.setFixedSize(self.THUMBNAIL_SIZE + 20, self.THUMBNAIL_SIZE + 50)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(5, 5, 5, 5)
        layout.setSpacing(5)
        
        # Thumbnail image
        self._image_label = QLabel()
        self._image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._image_label.setFixedSize(self.THUMBNAIL_SIZE, self.THUMBNAIL_SIZE)
        
        thumbnail = self._screenshot.thumbnail(self.THUMBNAIL_SIZE)
        self._image_label.setPixmap(thumbnail)
        
        layout.addWidget(self._image_label)
        
        # Info label
        self._info_label = QLabel()
        self._info_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._info_label.setText(self._screenshot.size_string())
        self._info_label.setStyleSheet("color: #cdd6f4; font-size: 11px;")
        
        layout.addWidget(self._info_label)
    
    def _apply_style(self):
        """Apply Catppuccin Mocha styling"""
        base_style = """
            ThumbnailWidget {
                background-color: #313244;
                border: 2px solid #45475a;
                border-radius: 8px;
            }
            ThumbnailWidget:hover {
                border-color: #89b4fa;
                background-color: #45475a;
            }
        """
        self.setStyleSheet(base_style)
    
    def set_selected(self, selected: bool):
        """Set selection state"""
        self._selected = selected
        
        if selected:
            self.setStyleSheet("""
                ThumbnailWidget {
                    background-color: #45475a;
                    border: 2px solid #89b4fa;
                    border-radius: 8px;
                }
            """)
        else:
            self._apply_style()
    
    @property
    def screenshot_id(self) -> UUID:
        return self._screenshot.id
    
    @property
    def screenshot(self) -> Screenshot:
        return self._screenshot
    
    def mousePressEvent(self, event: QMouseEvent):
        """Handle mouse click"""
        if event.button() == Qt.MouseButton.LeftButton:
            self.clicked.emit(self._screenshot.id)
        super().mousePressEvent(event)
    
    def mouseDoubleClickEvent(self, event: QMouseEvent):
        """Handle double-click - copy to clipboard"""
        if event.button() == Qt.MouseButton.LeftButton:
            self.double_clicked.emit(self._screenshot.id)
        super().mouseDoubleClickEvent(event)
    
    def contextMenuEvent(self, event: QContextMenuEvent):
        """Show context menu"""
        menu = QMenu(self)
        menu.setStyleSheet("""
            QMenu {
                background-color: #1e1e2e;
                color: #cdd6f4;
                border: 1px solid #45475a;
                border-radius: 4px;
                padding: 4px;
            }
            QMenu::item {
                padding: 6px 20px;
                border-radius: 4px;
            }
            QMenu::item:selected {
                background-color: #45475a;
            }
        """)
        
        copy_action = menu.addAction("📋 Copier")
        save_action = menu.addAction("💾 Sauvegarder")
        menu.addSeparator()
        delete_action = menu.addAction("🗑️ Supprimer")
        
        action = menu.exec(event.globalPos())
        
        if action == copy_action:
            self.copy_requested.emit(self._screenshot.id)
        elif action == save_action:
            self.save_requested.emit(self._screenshot.id)
        elif action == delete_action:
            self.delete_requested.emit(self._screenshot.id)
