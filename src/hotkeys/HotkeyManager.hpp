#pragma once

/**
 * Global Hotkey Manager
 *
 * Enables registering global keyboard shortcuts that work
 * even when the application is not focused.
 */

#include <QObject>
#include <QString>
#include <functional>
#include <map>
#include <memory>

namespace ShareY {

/**
 * Abstract hotkey backend
 */
class HotkeyBackend;

/**
 * Global hotkey manager
 *
 * Usage:
 *   HotkeyManager mgr;
 *   mgr.registerHotkey("PrintScreen", []() { ... });
 *   mgr.start();
 */
class HotkeyManager : public QObject {
  Q_OBJECT

public:
  explicit HotkeyManager(QObject *parent = nullptr);
  ~HotkeyManager() override;

  /**
   * Register a hotkey with a callback
   * @param keySequence Qt key sequence string (e.g., "Ctrl+Shift+S",
   * "PrintScreen")
   * @param callback Function to call when hotkey is pressed
   * @return true if registration successful
   */
  bool registerHotkey(const QString &keySequence,
                      std::function<void()> callback);

  /**
   * Unregister a hotkey
   * @param keySequence The key sequence to unregister
   */
  void unregisterHotkey(const QString &keySequence);

  /**
   * Unregister all hotkeys
   */
  void unregisterAll();

  /**
   * Start listening for hotkeys
   * Must be called after registering hotkeys
   */
  void start();

  /**
   * Stop listening for hotkeys
   */
  void stop();

  /**
   * Check if manager is running
   */
  bool isRunning() const { return m_running; }

signals:
  /**
   * Emitted when a registered hotkey is pressed
   */
  void hotkeyPressed(const QString &keySequence);

  /**
   * Emitted on registration error
   */
  void error(const QString &message);

private:
  std::unique_ptr<HotkeyBackend> m_backend;
  std::map<QString, std::function<void()>> m_callbacks;
  bool m_running{false};

  friend class HotkeyBackend;
  void onHotkeyPressed(const QString &keySequence);
};

} // namespace ShareY
