"""
Image Preview Dialog

A maximized overlay to display a screenshot at full size.
Click anywhere to close.
"""

from PyQt6.QtWidgets import QDialog, QLabel, QVBoxLayout, QApplication
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QPixmap, QKeyEvent, QMouseEvent


class ImagePreview(QDialog):
    """
    Maximized image preview overlay
    Click anywhere or press Escape to close
    """
    
    def __init__(self, pixmap: QPixmap, parent=None):
        super().__init__(parent)
        
        self._original_pixmap = pixmap
        
        self._setup_ui()
        self._apply_style()
    
    def _setup_ui(self):
        """Set up the preview UI"""
        self.setWindowTitle("Aperçu - Cliquez pour fermer")
        self.setModal(True)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        
        # Layout
        layout = QVBoxLayout(self)
        layout.setContentsMargins(10, 10, 10, 10)
        layout.setSpacing(5)
        
        # Image label
        self._image_label = QLabel()
        self._image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._image_label.setMinimumSize(400, 300)
        layout.addWidget(self._image_label, 1)
        
        # Info label
        self._info_label = QLabel("Cliquez n'importe où ou appuyez Échap pour fermer")
        self._info_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._info_label.setStyleSheet("""
            color: #a6adc8;
            font-size: 12px;
            padding: 5px;
        """)
        layout.addWidget(self._info_label)
        
        # Size info
        size_text = f"Taille originale: {self._original_pixmap.width()} x {self._original_pixmap.height()}"
        self._size_label = QLabel(size_text)
        self._size_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._size_label.setStyleSheet("color: #6c7086; font-size: 11px;")
        layout.addWidget(self._size_label)
    
    def showEvent(self, event):
        """Handle show event to update image after window is shown"""
        super().showEvent(event)
        # Delay image update to after window is fully shown
        QTimer.singleShot(50, self._update_image)
    
    def _update_image(self):
        """Scale image to fit the available space"""
        if self._original_pixmap.isNull():
            return
        
        # Get available size from label
        label_size = self._image_label.size()
        available_width = max(label_size.width() - 20, 400)
        available_height = max(label_size.height() - 20, 300)
        
        # Scale to fit
        scaled_pixmap = self._original_pixmap.scaled(
            available_width,
            available_height,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        )
        
        self._image_label.setPixmap(scaled_pixmap)
    
    def _apply_style(self):
        """Apply dark overlay style"""
        self.setStyleSheet("""
            QDialog {
                background-color: #11111b;
            }
            QLabel {
                background-color: transparent;
            }
        """)
    
    def resizeEvent(self, event):
        """Handle resize to update image scaling"""
        super().resizeEvent(event)
        QTimer.singleShot(10, self._update_image)
    
    def mousePressEvent(self, event: QMouseEvent):
        """Close on any click"""
        self.accept()
    
    def keyPressEvent(self, event: QKeyEvent):
        """Close on Escape"""
        if event.key() == Qt.Key.Key_Escape:
            self.accept()
        else:
            super().keyPressEvent(event)
    
    @staticmethod
    def show_preview(pixmap: QPixmap, parent=None):
        """Static method to show a preview dialog"""
        dialog = ImagePreview(pixmap, parent)
        dialog.showMaximized()
        dialog.exec()

