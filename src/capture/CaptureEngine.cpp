#include "CaptureEngine.hpp"
#include "X11Capture.hpp"
// #include "WaylandCapture.hpp"  // TODO: Implement

#include <cstdlib>

namespace ShareY {

std::unique_ptr<CaptureEngine> CaptureEngine::create() {
  // Check session type
  const char *sessionType = std::getenv("XDG_SESSION_TYPE");

  // Try X11 first (works in most cases, even on Wayland with XWayland)
  auto x11 = std::make_unique<X11Capture>();
  if (x11->initialize()) {
    return x11;
  }

  // TODO: Try Wayland portal
  // if (sessionType && std::string(sessionType) == "wayland") {
  //     auto wayland = std::make_unique<WaylandCapture>();
  //     if (wayland->initialize()) {
  //         return wayland;
  //     }
  // }

  // Fallback: return uninitialized X11 engine
  // Caller should check isReady()
  return x11;
}

} // namespace ShareY
