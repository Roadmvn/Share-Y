#pragma once

/**
 * Screenshot data structure
 *
 * Holds the image data in memory along with metadata.
 * Uses QPixmap for efficient rendering and QImage for manipulation.
 */

#include <QDateTime>
#include <QPixmap>
#include <QString>
#include <QUuid>

namespace ShareY {

/**
 * Capture type enumeration
 */
enum class CaptureType { Fullscreen, Region, Window, ActiveWindow };

/**
 * Screenshot container
 *
 * Stores image data in RAM with associated metadata.
 * Move-only semantics to prevent unnecessary copies.
 */
struct Screenshot {
  QUuid id;                   // Unique identifier
  QPixmap pixmap;             // The actual image data (GPU-backed)
  QDateTime timestamp;        // When the capture was taken
  CaptureType type;           // How it was captured
  QString windowTitle;        // If window capture, the title
  QSize originalSize;         // Original dimensions
  bool hasAnnotations{false}; // Whether annotations were added

  /**
   * Create a new screenshot from a pixmap
   */
  static Screenshot create(QPixmap &&pix, CaptureType captureType,
                           const QString &title = QString()) {
    Screenshot s;
    s.id = QUuid::createUuid();
    s.originalSize = pix.size();
    s.pixmap = std::move(pix);
    s.timestamp = QDateTime::currentDateTime();
    s.type = captureType;
    s.windowTitle = title;
    return s;
  }

  /**
   * Generate a thumbnail for display
   * @param maxSize Maximum dimension (width or height)
   */
  QPixmap thumbnail(int maxSize = 200) const {
    return pixmap.scaled(maxSize, maxSize, Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
  }

  /**
   * Get human-readable size
   */
  QString sizeString() const {
    return QString("%1 x %2")
        .arg(originalSize.width())
        .arg(originalSize.height());
  }

  /**
   * Estimate memory usage in bytes
   */
  size_t memoryUsage() const {
    // QPixmap memory = width * height * depth
    return static_cast<size_t>(pixmap.width()) *
           static_cast<size_t>(pixmap.height()) * (pixmap.depth() / 8);
  }

  // Move-only semantics
  Screenshot() = default;
  Screenshot(Screenshot &&) = default;
  Screenshot &operator=(Screenshot &&) = default;
  Screenshot(const Screenshot &) = delete;
  Screenshot &operator=(const Screenshot &) = delete;
};

} // namespace ShareY
