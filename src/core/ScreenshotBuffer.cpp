#include "ScreenshotBuffer.hpp"
#include <algorithm>

namespace ShareY {

ScreenshotBuffer::ScreenshotBuffer(QObject *parent) : QObject(parent) {}

void ScreenshotBuffer::add(Screenshot &&screenshot) {
  // Add at the beginning (newest first)
  m_screenshots.insert(m_screenshots.begin(), std::move(screenshot));

  const QUuid id = m_screenshots.front().id;

  // Enforce memory limits
  enforceMemoryLimit();

  emit screenshotAdded(id);
  emit memoryUsageChanged(totalMemoryUsage(), m_memoryLimit);
}

bool ScreenshotBuffer::remove(const QUuid &id) {
  auto it = std::find_if(m_screenshots.begin(), m_screenshots.end(),
                         [&id](const Screenshot &s) { return s.id == id; });

  if (it != m_screenshots.end()) {
    m_screenshots.erase(it);
    emit screenshotRemoved(id);
    emit memoryUsageChanged(totalMemoryUsage(), m_memoryLimit);
    return true;
  }
  return false;
}

const Screenshot *ScreenshotBuffer::get(const QUuid &id) const {
  auto it = std::find_if(m_screenshots.begin(), m_screenshots.end(),
                         [&id](const Screenshot &s) { return s.id == id; });

  return (it != m_screenshots.end()) ? &(*it) : nullptr;
}

Screenshot *ScreenshotBuffer::getMutable(const QUuid &id) {
  auto it = std::find_if(m_screenshots.begin(), m_screenshots.end(),
                         [&id](const Screenshot &s) { return s.id == id; });

  return (it != m_screenshots.end()) ? &(*it) : nullptr;
}

const Screenshot *ScreenshotBuffer::at(size_t index) const {
  if (index < m_screenshots.size()) {
    return &m_screenshots[index];
  }
  return nullptr;
}

void ScreenshotBuffer::clear() {
  m_screenshots.clear();
  emit bufferCleared();
  emit memoryUsageChanged(0, m_memoryLimit);
}

size_t ScreenshotBuffer::totalMemoryUsage() const {
  size_t total = 0;
  for (const auto &s : m_screenshots) {
    total += s.memoryUsage();
  }
  return total;
}

void ScreenshotBuffer::setMemoryLimit(size_t bytes) {
  m_memoryLimit = bytes;
  enforceMemoryLimit();
}

void ScreenshotBuffer::forEach(
    const std::function<void(const Screenshot &)> &callback) const {
  for (const auto &s : m_screenshots) {
    callback(s);
  }
}

std::vector<QUuid> ScreenshotBuffer::allIds() const {
  std::vector<QUuid> ids;
  ids.reserve(m_screenshots.size());
  for (const auto &s : m_screenshots) {
    ids.push_back(s.id);
  }
  return ids;
}

void ScreenshotBuffer::enforceMemoryLimit() {
  // Remove oldest screenshots (at the end) until under limit
  while (!m_screenshots.empty() && totalMemoryUsage() > m_memoryLimit) {
    QUuid removedId = m_screenshots.back().id;
    m_screenshots.pop_back();
    emit screenshotRemoved(removedId);
  }
}

} // namespace ShareY
