#pragma once

/**
 * Abstract Capture Engine Interface
 *
 * Defines the interface for screen capture backends.
 * Implementations: X11Capture (XCB), WaylandCapture (portal)
 */

#include <QPixmap>
#include <QRect>
#include <QString>
#include <memory>
#include <vector>

namespace ShareY {

/**
 * Window information for window capture
 */
struct WindowInfo {
  uint32_t id;       // Platform-specific window ID
  QString title;     // Window title
  QString className; // WM_CLASS
  QRect geometry;    // Window geometry
  bool isMinimized;
};

/**
 * Monitor information for multi-monitor support
 */
struct MonitorInfo {
  int index;
  QString name;
  QRect geometry;
  bool isPrimary;
};

/**
 * Abstract capture engine interface
 */
class CaptureEngine {
public:
  virtual ~CaptureEngine() = default;

  /**
   * Initialize the capture engine
   * @return true if initialization successful
   */
  virtual bool initialize() = 0;

  /**
   * Cleanup resources
   */
  virtual void cleanup() = 0;

  /**
   * Check if engine is ready
   */
  virtual bool isReady() const = 0;

  /**
   * Capture entire screen (all monitors)
   */
  virtual QPixmap captureFullscreen() = 0;

  /**
   * Capture a specific monitor
   * @param monitorIndex Which monitor to capture (0-indexed)
   */
  virtual QPixmap captureMonitor(int monitorIndex) = 0;

  /**
   * Capture a rectangular region
   * @param region Screen coordinates of the region
   */
  virtual QPixmap captureRegion(const QRect &region) = 0;

  /**
   * Capture a specific window
   * @param windowId Platform-specific window identifier
   */
  virtual QPixmap captureWindow(uint32_t windowId) = 0;

  /**
   * Capture the currently active window
   */
  virtual QPixmap captureActiveWindow() = 0;

  /**
   * Get list of available monitors
   */
  virtual std::vector<MonitorInfo> getMonitors() = 0;

  /**
   * Get list of visible windows
   */
  virtual std::vector<WindowInfo> getWindows() = 0;

  /**
   * Get engine name for debugging
   */
  virtual QString engineName() const = 0;

  /**
   * Factory: Create the best available capture engine
   * Tries X11 first, then Wayland portal
   */
  static std::unique_ptr<CaptureEngine> create();
};

} // namespace ShareY
