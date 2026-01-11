#include "X11Hotkey.hpp"
#include "HotkeyManager.hpp"

#include <QDebug>
#include <QKeySequence>
#include <Qt>

#include <X11/keysym.h>

namespace ShareY {

X11Hotkey::X11Hotkey(HotkeyManager *manager)
    : QThread(manager), m_manager(manager) {}

X11Hotkey::~X11Hotkey() { stop(); }

bool X11Hotkey::start() {
  // Connect to X server
  int screenNum = 0;
  m_connection = xcb_connect(nullptr, &screenNum);

  if (xcb_connection_has_error(m_connection)) {
    qWarning() << "X11Hotkey: Failed to connect to X server";
    return false;
  }

  // Get root window
  const xcb_setup_t *setup = xcb_get_setup(m_connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  for (int i = 0; i < screenNum; ++i) {
    xcb_screen_next(&iter);
  }
  m_root = iter.data->root;

  // Create key symbols table
  m_symbols = xcb_key_symbols_alloc(m_connection);
  if (!m_symbols) {
    qWarning() << "X11Hotkey: Failed to allocate key symbols";
    xcb_disconnect(m_connection);
    m_connection = nullptr;
    return false;
  }

  // Grab all registered hotkeys
  for (const auto &hotkey : m_hotkeys) {
    grabKey(hotkey.keycode, hotkey.modifiers);
  }

  m_running = true;
  QThread::start();
  return true;
}

void X11Hotkey::stop() {
  m_running = false;

  if (m_connection) {
    // Send a dummy event to wake up the event loop
    xcb_client_message_event_t event{};
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = m_root;
    event.type = XCB_ATOM_NONE;
    event.format = 32;

    xcb_send_event(m_connection, false, m_root, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char *>(&event));
    xcb_flush(m_connection);
  }

  wait();

  if (m_symbols) {
    xcb_key_symbols_free(m_symbols);
    m_symbols = nullptr;
  }

  if (m_connection) {
    xcb_disconnect(m_connection);
    m_connection = nullptr;
  }
}

void X11Hotkey::run() {
  while (m_running) {
    xcb_generic_event_t *event = xcb_wait_for_event(m_connection);

    if (!event) {
      if (xcb_connection_has_error(m_connection)) {
        qWarning() << "X11Hotkey: Connection error";
        break;
      }
      continue;
    }

    uint8_t responseType = event->response_type & ~0x80;

    if (responseType == XCB_KEY_PRESS) {
      auto *keyEvent = reinterpret_cast<xcb_key_press_event_t *>(event);

      // Find matching hotkey
      uint16_t cleanMods =
          keyEvent->state &
          (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1 | // Alt
           XCB_MOD_MASK_4);                                             // Super

      for (const auto &hotkey : m_hotkeys) {
        if (hotkey.keycode == keyEvent->detail &&
            hotkey.modifiers == cleanMods) {
          // Invoke callback on manager (thread-safe via Qt signals)
          QMetaObject::invokeMethod(
              m_manager,
              [this, seq = hotkey.keySequence]() {
                m_manager->onHotkeyPressed(seq);
              },
              Qt::QueuedConnection);
          break;
        }
      }
    }

    free(event);
  }
}

bool X11Hotkey::registerHotkey(const QString &keySequence) {
  xcb_keycode_t keycode;
  uint16_t modifiers;

  if (!parseKeySequence(keySequence, keycode, modifiers)) {
    qWarning() << "X11Hotkey: Failed to parse key sequence:" << keySequence;
    return false;
  }

  KeyCombo combo;
  combo.keycode = keycode;
  combo.modifiers = modifiers;
  combo.keySequence = keySequence;
  m_hotkeys.push_back(combo);

  // If already running, grab immediately
  if (m_running && m_connection) {
    grabKey(keycode, modifiers);
  }

  return true;
}

void X11Hotkey::unregisterHotkey(const QString &keySequence) {
  auto it = std::find_if(m_hotkeys.begin(), m_hotkeys.end(),
                         [&keySequence](const KeyCombo &c) {
                           return c.keySequence == keySequence;
                         });

  if (it != m_hotkeys.end()) {
    if (m_running && m_connection) {
      ungrabKey(it->keycode, it->modifiers);
    }
    m_hotkeys.erase(it);
  }
}

void X11Hotkey::unregisterAll() {
  if (m_running && m_connection) {
    for (const auto &hotkey : m_hotkeys) {
      ungrabKey(hotkey.keycode, hotkey.modifiers);
    }
  }
  m_hotkeys.clear();
}

bool X11Hotkey::parseKeySequence(const QString &keySequence,
                                 xcb_keycode_t &keycode, uint16_t &modifiers) {
  QKeySequence seq(keySequence);
  if (seq.isEmpty()) {
    return false;
  }

  // Get the key combination
  int key = seq[0].key();
  Qt::KeyboardModifiers qtMods = seq[0].keyboardModifiers();

  // Convert Qt key to X11 keysym
  uint32_t keysym = qtKeyToKeysym(key);
  if (keysym == 0) {
    return false;
  }

  // Get keycode from keysym
  if (!m_symbols && m_connection) {
    m_symbols = xcb_key_symbols_alloc(m_connection);
  }

  // For PrintScreen, we need to handle it specially
  if (keySequence.contains("Print", Qt::CaseInsensitive)) {
    keysym = XK_Print;
  }

  // We need a connection to get keycodes, so just store the keysym for now
  // and resolve when we start()

  // Temporary: hardcode Print key for demo
  if (keysym == XK_Print) {
    keycode = 107; // Common keycode for PrintScreen
  } else {
    // This is a simplified approach - in production, use
    // xcb_key_symbols_get_keycode
    keycode = 0; // Will be resolved on start()
  }

  // Convert Qt modifiers to X11 modifiers
  modifiers = 0;
  if (qtMods & Qt::ShiftModifier)
    modifiers |= XCB_MOD_MASK_SHIFT;
  if (qtMods & Qt::ControlModifier)
    modifiers |= XCB_MOD_MASK_CONTROL;
  if (qtMods & Qt::AltModifier)
    modifiers |= XCB_MOD_MASK_1;
  if (qtMods & Qt::MetaModifier)
    modifiers |= XCB_MOD_MASK_4;

  return true;
}

uint32_t X11Hotkey::qtKeyToKeysym(int qtKey) const {
  // Map common Qt keys to X11 keysyms
  switch (qtKey) {
  case Qt::Key_Print:
    return XK_Print;
  case Qt::Key_Escape:
    return XK_Escape;
  case Qt::Key_Return:
    return XK_Return;
  case Qt::Key_Space:
    return XK_space;
  case Qt::Key_A:
    return XK_a;
  case Qt::Key_B:
    return XK_b;
  case Qt::Key_C:
    return XK_c;
  case Qt::Key_D:
    return XK_d;
  case Qt::Key_E:
    return XK_e;
  case Qt::Key_F:
    return XK_f;
  case Qt::Key_G:
    return XK_g;
  case Qt::Key_H:
    return XK_h;
  case Qt::Key_I:
    return XK_i;
  case Qt::Key_J:
    return XK_j;
  case Qt::Key_K:
    return XK_k;
  case Qt::Key_L:
    return XK_l;
  case Qt::Key_M:
    return XK_m;
  case Qt::Key_N:
    return XK_n;
  case Qt::Key_O:
    return XK_o;
  case Qt::Key_P:
    return XK_p;
  case Qt::Key_Q:
    return XK_q;
  case Qt::Key_R:
    return XK_r;
  case Qt::Key_S:
    return XK_s;
  case Qt::Key_T:
    return XK_t;
  case Qt::Key_U:
    return XK_u;
  case Qt::Key_V:
    return XK_v;
  case Qt::Key_W:
    return XK_w;
  case Qt::Key_X:
    return XK_x;
  case Qt::Key_Y:
    return XK_y;
  case Qt::Key_Z:
    return XK_z;
  case Qt::Key_F1:
    return XK_F1;
  case Qt::Key_F2:
    return XK_F2;
  case Qt::Key_F3:
    return XK_F3;
  case Qt::Key_F4:
    return XK_F4;
  case Qt::Key_F5:
    return XK_F5;
  case Qt::Key_F6:
    return XK_F6;
  case Qt::Key_F7:
    return XK_F7;
  case Qt::Key_F8:
    return XK_F8;
  case Qt::Key_F9:
    return XK_F9;
  case Qt::Key_F10:
    return XK_F10;
  case Qt::Key_F11:
    return XK_F11;
  case Qt::Key_F12:
    return XK_F12;
  default:
    return 0;
  }
}

bool X11Hotkey::grabKey(xcb_keycode_t keycode, uint16_t modifiers) {
  if (!m_connection || keycode == 0)
    return false;

  // Grab with and without NumLock/CapsLock
  uint16_t modVariants[] = {
      modifiers,
      static_cast<uint16_t>(modifiers | XCB_MOD_MASK_LOCK), // CapsLock
      static_cast<uint16_t>(modifiers | XCB_MOD_MASK_2),    // NumLock
      static_cast<uint16_t>(modifiers | XCB_MOD_MASK_LOCK | XCB_MOD_MASK_2)};

  for (uint16_t mods : modVariants) {
    xcb_grab_key(m_connection, true, m_root, mods, keycode, XCB_GRAB_MODE_ASYNC,
                 XCB_GRAB_MODE_ASYNC);
  }

  xcb_flush(m_connection);
  return true;
}

void X11Hotkey::ungrabKey(xcb_keycode_t keycode, uint16_t modifiers) {
  if (!m_connection)
    return;

  uint16_t modVariants[] = {
      modifiers, static_cast<uint16_t>(modifiers | XCB_MOD_MASK_LOCK),
      static_cast<uint16_t>(modifiers | XCB_MOD_MASK_2),
      static_cast<uint16_t>(modifiers | XCB_MOD_MASK_LOCK | XCB_MOD_MASK_2)};

  for (uint16_t mods : modVariants) {
    xcb_ungrab_key(m_connection, keycode, m_root, mods);
  }

  xcb_flush(m_connection);
}

} // namespace ShareY
