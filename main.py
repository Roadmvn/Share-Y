#!/usr/bin/env python3
"""
ShareY - Linux Screenshot Manager

A lightweight, performant screenshot tool inspired by ShareX.
Stores captures in RAM without automatic disk writes.

Author: Tudy
License: MIT
"""

import sys
import signal

from PyQt6.QtWidgets import QApplication, QSystemTrayIcon, QMenu, QMessageBox
from PyQt6.QtGui import QIcon, QAction

from src.ui.main_window import MainWindow
from src.hotkeys.manager import HotkeyManager


def main():
    # Handle Ctrl+C gracefully
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    
    app = QApplication(sys.argv)
    
    # Application metadata
    app.setApplicationName("Share-Y")
    app.setApplicationVersion("1.0.0")
    app.setOrganizationName("ShareY")
    app.setQuitOnLastWindowClosed(False)  # Keep running in tray
    
    # Check for system tray support
    if not QSystemTrayIcon.isSystemTrayAvailable():
        QMessageBox.critical(
            None,
            "Share-Y",
            "System tray not available. Share-Y requires a system tray to run."
        )
        return 1
    
    # Apply global dark theme
    app.setStyleSheet("""
        QToolTip {
            background-color: #1e1e2e;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 4px;
        }
        QMessageBox {
            background-color: #1e1e2e;
        }
        QMessageBox QLabel {
            color: #cdd6f4;
        }
        QMessageBox QPushButton {
            background-color: #313244;
            color: #cdd6f4;
            border: none;
            border-radius: 4px;
            padding: 6px 16px;
            min-width: 80px;
        }
        QMessageBox QPushButton:hover {
            background-color: #45475a;
        }
    """)
    
    # Create main window
    main_window = MainWindow()
    
    # Create system tray icon
    tray_icon = QSystemTrayIcon()
    tray_icon.setIcon(QIcon.fromTheme("camera-photo", QIcon.fromTheme("applets-screenshooter")))
    tray_icon.setToolTip("Share-Y - Screenshot Manager")
    
    # Tray menu
    tray_menu = QMenu()
    tray_menu.setStyleSheet("""
        QMenu {
            background-color: #1e1e2e;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 6px;
        }
        QMenu::item {
            padding: 8px 24px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: #45475a;
        }
        QMenu::separator {
            height: 1px;
            background-color: #45475a;
            margin: 4px 8px;
        }
    """)
    
    # Show Dashboard action
    show_action = QAction("📊 Afficher le Dashboard", tray_menu)
    show_action.triggered.connect(lambda: (
        main_window.show(),
        main_window.raise_(),
        main_window.activateWindow()
    ))
    tray_menu.addAction(show_action)
    
    tray_menu.addSeparator()
    
    # Capture Fullscreen action
    capture_full_action = QAction("📷 Capture Plein Écran (Ctrl+Alt+P)", tray_menu)
    capture_full_action.triggered.connect(main_window.capture_fullscreen)
    tray_menu.addAction(capture_full_action)
    
    # Capture Region action
    capture_region_action = QAction("✂️ Capture Région (Ctrl+Alt+R)", tray_menu)
    capture_region_action.triggered.connect(main_window.capture_region)
    tray_menu.addAction(capture_region_action)
    
    tray_menu.addSeparator()
    
    # Quit action
    quit_action = QAction("❌ Quitter", tray_menu)
    quit_action.triggered.connect(app.quit)
    tray_menu.addAction(quit_action)
    
    tray_icon.setContextMenu(tray_menu)
    
    # Double-click to show window
    def on_tray_activated(reason):
        if reason == QSystemTrayIcon.ActivationReason.DoubleClick:
            main_window.show()
            main_window.raise_()
            main_window.activateWindow()
    
    tray_icon.activated.connect(on_tray_activated)
    tray_icon.show()
    
    # Initialize global hotkey manager
    hotkey_manager = HotkeyManager()
    
    # Register default hotkeys (different from system defaults to avoid conflicts)
    hotkey_manager.register_hotkey("Ctrl+Alt+P", main_window.capture_fullscreen)
    hotkey_manager.register_hotkey("Ctrl+Alt+R", main_window.capture_region)
    
    hotkey_manager.start()
    
    # Show main window on first launch
    main_window.show()
    
    print("Share-Y started!")
    print("Hotkeys:")
    print("  Ctrl+Alt+P : Capture fullscreen")
    print("  Ctrl+Alt+R : Capture region")
    
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
