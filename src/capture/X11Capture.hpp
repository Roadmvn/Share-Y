#pragma once

/**
 * X11 Capture Engine using XCB
 *
 * Uses XCB (X C Binding) for efficient X11 screen capture.
 * XCB is more modern and performant than Xlib.
 */

#include "CaptureEngine.hpp"

#include <xcb/shm.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

namespace ShareY {

class X11Capture : public CaptureEngine {
public:
  X11Capture();
  ~X11Capture() override;

  // Prevent copying
  X11Capture(const X11Capture &) = delete;
  X11Capture &operator=(const X11Capture &) = delete;

  // CaptureEngine interface
  bool initialize() override;
  void cleanup() override;
  bool isReady() const override { return m_connection != nullptr; }

  QPixmap captureFullscreen() override;
  QPixmap captureMonitor(int monitorIndex) override;
  QPixmap captureRegion(const QRect &region) override;
  QPixmap captureWindow(uint32_t windowId) override;
  QPixmap captureActiveWindow() override;

  std::vector<MonitorInfo> getMonitors() override;
  std::vector<WindowInfo> getWindows() override;

  QString engineName() const override { return "X11 (XCB)"; }

private:
  /**
   * Capture a region of the screen using XCB
   * Core capture function used by all public methods
   */
  QPixmap captureRect(xcb_drawable_t drawable, const QRect &rect);

  /**
   * Get the root window
   */
  xcb_window_t rootWindow() const;

  /**
   * Get screen geometry
   */
  QRect screenGeometry() const;

  /**
   * Get active window using EWMH
   */
  xcb_window_t getActiveWindow() const;

  /**
   * Get window title using EWMH/ICCCM
   */
  QString getWindowTitle(xcb_window_t window) const;

  /**
   * Check if SHM extension is available
   */
  bool checkShmExtension();

  xcb_connection_t *m_connection{nullptr};
  xcb_screen_t *m_screen{nullptr};
  bool m_useSHM{false};

  // Cached atoms for EWMH
  xcb_atom_t m_atomNetActiveWindow{XCB_ATOM_NONE};
  xcb_atom_t m_atomNetWmName{XCB_ATOM_NONE};
  xcb_atom_t m_atomUtf8String{XCB_ATOM_NONE};
  xcb_atom_t m_atomWmName{XCB_ATOM_NONE};
  xcb_atom_t m_atomNetClientList{XCB_ATOM_NONE};
};

} // namespace ShareY
