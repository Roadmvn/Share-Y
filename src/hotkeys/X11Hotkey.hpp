#pragma once

/**
 * X11 Global Hotkey Backend
 *
 * Uses XCB to grab global hotkeys via XGrabKey.
 * Runs a separate event loop in a thread to avoid blocking Qt.
 */

#include <QString>
#include <QThread>
#include <map>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "HotkeyManager.hpp"

namespace ShareY {

class HotkeyManager;

/**
 * X11-specific hotkey implementation
 */
class X11Hotkey : public QThread, public HotkeyBackend {
  Q_OBJECT

public:
  explicit X11Hotkey(HotkeyManager *manager);
  ~X11Hotkey() override;

  /**
   * Register a hotkey
   * @param keySequence Qt-style key sequence string
   */
  bool registerHotkey(const QString &keySequence);

  /**
   * Unregister a hotkey
   */
  void unregisterHotkey(const QString &keySequence);

  /**
   * Unregister all hotkeys
   */
  void unregisterAll();

  /**
   * Start the event loop
   */
  bool start();

  /**
   * Stop the event loop
   */
  void stop();

protected:
  void run() override;

private:
  /**
   * Parse Qt key sequence to X11 keycode and modifiers
   */
  bool parseKeySequence(const QString &keySequence, xcb_keycode_t &keycode,
                        uint16_t &modifiers);

  /**
   * Convert Qt key to X11 keysym
   */
  uint32_t qtKeyToKeysym(int qtKey) const;

  /**
   * Grab a key combination
   */
  bool grabKey(xcb_keycode_t keycode, uint16_t modifiers);

  /**
   * Ungrab a key combination
   */
  void ungrabKey(xcb_keycode_t keycode, uint16_t modifiers);

  HotkeyManager *m_manager;
  xcb_connection_t *m_connection{nullptr};
  xcb_key_symbols_t *m_symbols{nullptr};
  xcb_window_t m_root{XCB_WINDOW_NONE};

  bool m_running{false};

  // Registered hotkeys: (keycode, modifiers) -> keySequence
  struct KeyCombo {
    xcb_keycode_t keycode;
    uint16_t modifiers;
    QString keySequence;
  };
  std::vector<KeyCombo> m_hotkeys;
};

} // namespace ShareY
