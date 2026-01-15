"""
Main Application Window

The dashboard that displays captured screenshots as thumbnails.
Provides actions like copy, save, delete, and edit.
"""

from typing import Dict, Optional
from uuid import UUID

from PyQt6.QtWidgets import (QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
                              QScrollArea, QLabel, QToolBar, QStatusBar,
                              QFileDialog, QApplication, QMessageBox)
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QAction, QIcon, QCloseEvent, QKeyEvent

from ..core.screenshot import Screenshot
from ..core.buffer import ScreenshotBuffer
from ..capture.capturer import Capturer
from .thumbnail import ThumbnailWidget
from .flow_layout import FlowLayout
from .preview import ImagePreview


class MainWindow(QMainWindow):
    """
    Main window with screenshot dashboard
    """
    
    def __init__(self):
        super().__init__()
        
        self._buffer = ScreenshotBuffer()
        self._capturer = Capturer()
        self._thumbnails: Dict[UUID, ThumbnailWidget] = {}
        self._selected_id: Optional[UUID] = None
        
        self._setup_ui()
        self._connect_signals()
        self._apply_style()
    
    def _setup_ui(self):
        """Set up the main window UI"""
        self.setWindowTitle("Share-Y 📸")
        self.setMinimumSize(800, 600)
        self.resize(1000, 700)
        
        # Central widget
        central = QWidget()
        self.setCentralWidget(central)
        
        main_layout = QVBoxLayout(central)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Toolbar
        self._setup_toolbar()
        
        # Header
        header = QWidget()
        header.setFixedHeight(60)
        header.setStyleSheet("background-color: #1e1e2e; border-bottom: 1px solid #313244;")
        header_layout = QHBoxLayout(header)
        
        title = QLabel("📸 Share-Y")
        title.setStyleSheet("font-size: 24px; font-weight: bold; color: #cdd6f4;")
        header_layout.addWidget(title)
        
        header_layout.addStretch()
        
        self._count_label = QLabel("0 captures")
        self._count_label.setStyleSheet("font-size: 14px; color: #a6adc8;")
        header_layout.addWidget(self._count_label)
        
        main_layout.addWidget(header)
        
        # Scroll area for thumbnails
        self._scroll_area = QScrollArea()
        self._scroll_area.setWidgetResizable(True)
        self._scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self._scroll_area.setStyleSheet("""
            QScrollArea {
                background-color: #1e1e2e;
                border: none;
            }
            QScrollBar:vertical {
                background-color: #1e1e2e;
                width: 10px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical {
                background-color: #45475a;
                border-radius: 5px;
                min-height: 30px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #585b70;
            }
        """)
        
        # Thumbnail container
        self._thumbnail_container = QWidget()
        self._thumbnail_container.setStyleSheet("background-color: #1e1e2e;")
        self._thumbnail_layout = FlowLayout(self._thumbnail_container, 20, 15, 15)
        
        self._scroll_area.setWidget(self._thumbnail_container)
        main_layout.addWidget(self._scroll_area)
        
        # Empty state label
        self._empty_label = QLabel("Aucune capture\n\nCtrl+Alt+P : Capture plein écran\nCtrl+Alt+R : Capture région")
        self._empty_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._empty_label.setStyleSheet("""
            font-size: 16px;
            color: #6c7086;
            padding: 50px;
        """)
        main_layout.addWidget(self._empty_label)
        
        # Status bar
        self._setup_statusbar()
    
    def _setup_toolbar(self):
        """Set up the toolbar"""
        toolbar = QToolBar()
        toolbar.setMovable(False)
        toolbar.setStyleSheet("""
            QToolBar {
                background-color: #181825;
                border-bottom: 1px solid #313244;
                padding: 5px;
                spacing: 10px;
            }
            QToolButton {
                background-color: #313244;
                color: #cdd6f4;
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
                font-size: 13px;
            }
            QToolButton:hover {
                background-color: #45475a;
            }
            QToolButton:pressed {
                background-color: #585b70;
            }
        """)
        
        # Capture fullscreen
        fullscreen_action = QAction("📷 Plein écran", self)
        fullscreen_action.setShortcut("Ctrl+Alt+P")
        fullscreen_action.triggered.connect(self.capture_fullscreen)
        toolbar.addAction(fullscreen_action)
        
        # Capture region
        region_action = QAction("✂️ Région", self)
        region_action.setShortcut("Ctrl+Alt+R")
        region_action.triggered.connect(self.capture_region)
        toolbar.addAction(region_action)
        
        toolbar.addSeparator()
        
        # Clear all
        clear_action = QAction("🗑️ Tout effacer", self)
        clear_action.triggered.connect(self._clear_all)
        toolbar.addAction(clear_action)
        
        self.addToolBar(toolbar)
    
    def _setup_statusbar(self):
        """Set up the status bar"""
        statusbar = QStatusBar()
        statusbar.setStyleSheet("""
            QStatusBar {
                background-color: #181825;
                color: #a6adc8;
                border-top: 1px solid #313244;
            }
        """)
        
        self._status_label = QLabel("Prêt")
        self._memory_label = QLabel("Mémoire: 0 B")
        
        statusbar.addWidget(self._status_label, 1)
        statusbar.addPermanentWidget(self._memory_label)
        
        self.setStatusBar(statusbar)
    
    def _connect_signals(self):
        """Connect buffer signals"""
        self._buffer.screenshot_added.connect(self._on_screenshot_added)
        self._buffer.screenshot_removed.connect(self._on_screenshot_removed)
        self._buffer.buffer_cleared.connect(self._on_buffer_cleared)
    
    def _apply_style(self):
        """Apply global Catppuccin Mocha style"""
        self.setStyleSheet("""
            QMainWindow {
                background-color: #1e1e2e;
            }
        """)
    
    def _update_ui(self):
        """Update UI state"""
        count = self._buffer.count()
        self._count_label.setText(f"{count} capture{'s' if count != 1 else ''}")
        self._memory_label.setText(f"Mémoire: {self._buffer.memory_usage_string()}")
        
        # Show/hide empty state
        has_screenshots = count > 0
        self._empty_label.setVisible(not has_screenshots)
        self._scroll_area.setVisible(has_screenshots)
    
    def capture_fullscreen(self):
        """Capture the entire screen"""
        self._status_label.setText("Capture en cours...")
        QApplication.processEvents()
        
        # Hide window during capture
        was_visible = self.isVisible()
        if was_visible:
            self.hide()
        
        # Small delay for window to hide
        QTimer.singleShot(200, lambda: self._do_capture_fullscreen(was_visible))
    
    def _do_capture_fullscreen(self, restore_visible: bool):
        """Perform the fullscreen capture"""
        screenshot = self._capturer.capture_fullscreen()
        
        if screenshot:
            self._buffer.add(screenshot)
            self._status_label.setText("Capture réussie!")
        else:
            self._status_label.setText("Capture annulée ou échouée")
        
        if restore_visible:
            self.show()
    
    def capture_region(self):
        """Capture a selected region"""
        self._status_label.setText("Sélectionnez une région...")
        QApplication.processEvents()
        
        # Hide window during capture
        was_visible = self.isVisible()
        if was_visible:
            self.hide()
        
        QTimer.singleShot(200, lambda: self._do_capture_region(was_visible))
    
    def _do_capture_region(self, restore_visible: bool):
        """Perform region capture"""
        screenshot = self._capturer.capture_region()
        
        if screenshot:
            self._buffer.add(screenshot)
            self._status_label.setText("Capture réussie!")
        else:
            self._status_label.setText("Capture annulée ou échouée")
        
        if restore_visible:
            self.show()
    
    def _on_screenshot_added(self, screenshot_id: UUID):
        """Handle new screenshot"""
        screenshot = self._buffer.get(screenshot_id)
        if not screenshot:
            return
        
        # Create thumbnail widget
        thumbnail = ThumbnailWidget(screenshot)
        thumbnail.clicked.connect(self._on_thumbnail_clicked)
        thumbnail.double_clicked.connect(self._copy_to_clipboard)
        thumbnail.copy_requested.connect(self._copy_to_clipboard)
        thumbnail.save_requested.connect(self._save_to_file)
        thumbnail.delete_requested.connect(self._delete_screenshot)
        
        self._thumbnails[screenshot_id] = thumbnail
        self._thumbnail_layout.addWidget(thumbnail)
        
        self._update_ui()
        
        # Force layout refresh with a small delay
        QTimer.singleShot(50, self._refresh_thumbnail_container)
    
    def _refresh_thumbnail_container(self):
        """Force refresh of thumbnail container layout"""
        # Calculate proper height based on layout
        width = self._scroll_area.viewport().width()
        height = self._thumbnail_layout.heightForWidth(width)
        
        # Set minimum height to show all thumbnails
        self._thumbnail_container.setMinimumHeight(height + 50)
        
        # Force updates
        self._thumbnail_layout.invalidate()
        self._thumbnail_layout.activate()
        self._thumbnail_container.updateGeometry()
        self._scroll_area.updateGeometry()
    
    def _on_screenshot_removed(self, screenshot_id: UUID):
        """Handle screenshot removal"""
        if screenshot_id in self._thumbnails:
            widget = self._thumbnails.pop(screenshot_id)
            self._thumbnail_layout.removeWidget(widget)
            widget.deleteLater()
        
        self._update_ui()
    
    def _on_buffer_cleared(self):
        """Handle buffer cleared"""
        for widget in self._thumbnails.values():
            widget.deleteLater()
        self._thumbnails.clear()
        self._thumbnail_layout.clear()
        self._update_ui()
    
    def _on_thumbnail_clicked(self, screenshot_id: UUID):
        """Handle thumbnail click - show full preview"""
        screenshot = self._buffer.get(screenshot_id)
        if not screenshot:
            return
        
        # Show fullscreen preview
        ImagePreview.show_preview(screenshot.pixmap, self)
    
    def _copy_to_clipboard(self, screenshot_id: UUID):
        """Copy screenshot to clipboard"""
        screenshot = self._buffer.get(screenshot_id)
        if not screenshot:
            return
        
        clipboard = QApplication.clipboard()
        clipboard.setPixmap(screenshot.pixmap)
        self._status_label.setText("Copié dans le presse-papiers!")
    
    def _save_to_file(self, screenshot_id: UUID):
        """Save screenshot to file"""
        screenshot = self._buffer.get(screenshot_id)
        if not screenshot:
            return
        
        filename, _ = QFileDialog.getSaveFileName(
            self,
            "Sauvegarder la capture",
            f"capture_{screenshot.timestamp.strftime('%Y%m%d_%H%M%S')}.png",
            "Images PNG (*.png);;Images JPEG (*.jpg);;Tous les fichiers (*)"
        )
        
        if filename:
            if screenshot.pixmap.save(filename):
                self._status_label.setText(f"Sauvegardé: {filename}")
            else:
                self._status_label.setText("Erreur lors de la sauvegarde")
    
    def _delete_screenshot(self, screenshot_id: UUID):
        """Delete a screenshot"""
        self._buffer.remove(screenshot_id)
        self._status_label.setText("Capture supprimée")
    
    def _clear_all(self):
        """Clear all screenshots"""
        if self._buffer.count() == 0:
            return
        
        reply = QMessageBox.question(
            self,
            "Confirmer",
            "Supprimer toutes les captures?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )
        
        if reply == QMessageBox.StandardButton.Yes:
            self._buffer.clear()
            self._status_label.setText("Toutes les captures supprimées")
    
    def closeEvent(self, event: QCloseEvent):
        """Handle window close - hide to tray instead"""
        event.ignore()
        self.hide()
    
    def keyPressEvent(self, event: QKeyEvent):
        """Handle key press"""
        if event.key() == Qt.Key.Key_Escape:
            self.hide()
        elif event.key() == Qt.Key.Key_Delete and self._selected_id:
            self._delete_screenshot(self._selected_id)
        else:
            super().keyPressEvent(event)
