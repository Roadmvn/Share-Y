/**
 * ShareY - Linux Screenshot Manager
 * 
 * A lightweight, performant screenshot tool inspired by ShareX.
 * Stores captures in RAM without automatic disk writes.
 * 
 * Author: Tudy
 * License: MIT
 */

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>

#include "ui/MainWindow.hpp"
#include "hotkeys/HotkeyManager.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Application metadata
    QApplication::setApplicationName("ShareY");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("ShareY");
    QApplication::setQuitOnLastWindowClosed(false);  // Keep running in tray
    
    // Check for system tray support
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "ShareY", 
            "System tray not available. ShareY requires a system tray to run.");
        return 1;
    }
    
    // Create main window
    ShareY::MainWindow mainWindow;
    
    // Create system tray icon
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/icons/shareY.png"));
    trayIcon.setToolTip("ShareY - Screenshot Manager");
    
    // Tray menu
    QMenu trayMenu;
    
    QAction* showAction = trayMenu.addAction("Show Dashboard");
    QObject::connect(showAction, &QAction::triggered, [&mainWindow]() {
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
    });
    
    QAction* captureFullAction = trayMenu.addAction("Capture Fullscreen");
    QObject::connect(captureFullAction, &QAction::triggered, [&mainWindow]() {
        mainWindow.captureFullscreen();
    });
    
    QAction* captureRegionAction = trayMenu.addAction("Capture Region");
    QObject::connect(captureRegionAction, &QAction::triggered, [&mainWindow]() {
        mainWindow.captureRegion();
    });
    
    trayMenu.addSeparator();
    
    QAction* quitAction = trayMenu.addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    
    trayIcon.setContextMenu(&trayMenu);
    
    // Double-click to show window
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, 
        [&mainWindow](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::DoubleClick) {
                mainWindow.show();
                mainWindow.raise();
                mainWindow.activateWindow();
            }
        });
    
    trayIcon.show();
    
    // Initialize global hotkey manager
    ShareY::HotkeyManager hotkeyManager;
    
    // Register default hotkeys
    hotkeyManager.registerHotkey("PrintScreen", [&mainWindow]() {
        mainWindow.captureFullscreen();
    });
    
    hotkeyManager.registerHotkey("Ctrl+PrintScreen", [&mainWindow]() {
        mainWindow.captureRegion();
    });
    
    hotkeyManager.start();
    
    // Show main window on first launch
    mainWindow.show();
    
    return app.exec();
}
