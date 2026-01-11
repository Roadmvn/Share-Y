#include "HotkeyManager.hpp"
#include "X11Hotkey.hpp"

#include <QDebug>

namespace ShareY {

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent), m_backend(std::make_unique<X11Hotkey>(this)) {}

HotkeyManager::~HotkeyManager() { stop(); }

bool HotkeyManager::registerHotkey(const QString &keySequence,
                                   std::function<void()> callback) {
  if (!m_backend) {
    emit error("No hotkey backend available");
    return false;
  }

  if (!m_backend->registerHotkey(keySequence)) {
    emit error(QString("Failed to register hotkey: %1").arg(keySequence));
    return false;
  }

  m_callbacks[keySequence] = std::move(callback);
  qInfo() << "Registered hotkey:" << keySequence;
  return true;
}

void HotkeyManager::unregisterHotkey(const QString &keySequence) {
  if (m_backend) {
    m_backend->unregisterHotkey(keySequence);
  }
  m_callbacks.erase(keySequence);
}

void HotkeyManager::unregisterAll() {
  if (m_backend) {
    m_backend->unregisterAll();
  }
  m_callbacks.clear();
}

void HotkeyManager::start() {
  if (m_running)
    return;

  if (m_backend && m_backend->start()) {
    m_running = true;
    qInfo() << "Hotkey manager started";
  } else {
    emit error("Failed to start hotkey backend");
  }
}

void HotkeyManager::stop() {
  if (!m_running)
    return;

  if (m_backend) {
    m_backend->stop();
  }
  m_running = false;
  qInfo() << "Hotkey manager stopped";
}

void HotkeyManager::onHotkeyPressed(const QString &keySequence) {
  emit hotkeyPressed(keySequence);

  auto it = m_callbacks.find(keySequence);
  if (it != m_callbacks.end() && it->second) {
    it->second();
  }
}

} // namespace ShareY
