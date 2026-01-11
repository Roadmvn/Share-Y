#include "CaptureEngine.hpp"
#include "X11Capture.hpp"
#include "PortalCapture.hpp"

#include <cstdlib>
#include <cstring>

namespace ShareY {

std::unique_ptr<CaptureEngine> CaptureEngine::create() {
  // Check session type
  const char *sessionType = std::getenv("XDG_SESSION_TYPE");

  // On Wayland, use Portal-based capture (gnome-screenshot/grim)
  if (sessionType && std::strcmp(sessionType, "wayland") == 0) {
    auto portal = std::make_unique<PortalCapture>();
    if (portal->initialize()) {
      return portal;
    }
  }

  // Try X11 (works on X11 and sometimes on XWayland)
  auto x11 = std::make_unique<X11Capture>();
  if (x11->initialize()) {
    return x11;
  }

  // Fallback: try Portal even on X11 if X11 capture failed
  auto portal = std::make_unique<PortalCapture>();
  if (portal->initialize()) {
    return portal;
  }

  // Return uninitialized engine - caller should check isReady()
  return x11;
}

} // namespace ShareY

