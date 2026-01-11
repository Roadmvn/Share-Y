#pragma once

/**
 * In-Memory Screenshot Buffer
 *
 * Stores screenshots in RAM without writing to disk.
 * Implements memory limits and automatic eviction of old captures.
 */

#include <QObject>
#include <QUuid>
#include <functional>
#include <optional>
#include <vector>

#include "Screenshot.hpp"

namespace ShareY {

/**
 * Thread-safe in-memory buffer for screenshots
 */
class ScreenshotBuffer : public QObject {
  Q_OBJECT

public:
  explicit ScreenshotBuffer(QObject *parent = nullptr);
  ~ScreenshotBuffer() override = default;

  /**
   * Add a screenshot to the buffer
   * May evict old screenshots if memory limit is exceeded
   */
  void add(Screenshot &&screenshot);

  /**
   * Remove a screenshot by ID
   * @return true if found and removed
   */
  bool remove(const QUuid &id);

  /**
   * Get screenshot by ID (const reference)
   */
  const Screenshot *get(const QUuid &id) const;

  /**
   * Get screenshot by ID (mutable, for editing)
   */
  Screenshot *getMutable(const QUuid &id);

  /**
   * Get screenshot by index
   */
  const Screenshot *at(size_t index) const;

  /**
   * Number of screenshots currently stored
   */
  size_t count() const { return m_screenshots.size(); }

  /**
   * Check if buffer is empty
   */
  bool isEmpty() const { return m_screenshots.empty(); }

  /**
   * Clear all screenshots
   */
  void clear();

  /**
   * Get total memory usage in bytes
   */
  size_t totalMemoryUsage() const;

  /**
   * Get memory limit in bytes
   */
  size_t memoryLimit() const { return m_memoryLimit; }

  /**
   * Set memory limit in bytes (default: 512 MB)
   * Will evict old screenshots if current usage exceeds new limit
   */
  void setMemoryLimit(size_t bytes);

  /**
   * Iterate over all screenshots
   */
  void forEach(const std::function<void(const Screenshot &)> &callback) const;

  /**
   * Get all screenshot IDs in order (newest first)
   */
  std::vector<QUuid> allIds() const;

signals:
  /**
   * Emitted when a screenshot is added
   */
  void screenshotAdded(const QUuid &id);

  /**
   * Emitted when a screenshot is removed
   */
  void screenshotRemoved(const QUuid &id);

  /**
   * Emitted when buffer is cleared
   */
  void bufferCleared();

  /**
   * Emitted when memory usage changes significantly
   */
  void memoryUsageChanged(size_t currentBytes, size_t limitBytes);

private:
  /**
   * Evict oldest screenshots until under memory limit
   */
  void enforceMemoryLimit();

  std::vector<Screenshot> m_screenshots;
  size_t m_memoryLimit{512 * 1024 * 1024}; // 512 MB default
};

} // namespace ShareY
