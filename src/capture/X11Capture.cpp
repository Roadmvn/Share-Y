#include "X11Capture.hpp"

#include <QDebug>
#include <QImage>
#include <cstring>

namespace ShareY {

X11Capture::X11Capture() = default;

X11Capture::~X11Capture() { cleanup(); }

bool X11Capture::initialize() {
  // Connect to X server
  int screenNum = 0;
  m_connection = xcb_connect(nullptr, &screenNum);

  if (xcb_connection_has_error(m_connection)) {
    qWarning() << "Failed to connect to X server";
    cleanup();
    return false;
  }

  // Get the screen
  const xcb_setup_t *setup = xcb_get_setup(m_connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

  for (int i = 0; i < screenNum; ++i) {
    xcb_screen_next(&iter);
  }
  m_screen = iter.data;

  if (!m_screen) {
    qWarning() << "Failed to get X11 screen";
    cleanup();
    return false;
  }

  // Check SHM extension for faster captures
  m_useSHM = checkShmExtension();

  // Cache EWMH atoms for window management
  auto internAtom = [this](const char *name) -> xcb_atom_t {
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(m_connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t *reply =
        xcb_intern_atom_reply(m_connection, cookie, nullptr);
    if (reply) {
      xcb_atom_t atom = reply->atom;
      free(reply);
      return atom;
    }
    return XCB_ATOM_NONE;
  };

  m_atomNetActiveWindow = internAtom("_NET_ACTIVE_WINDOW");
  m_atomNetWmName = internAtom("_NET_WM_NAME");
  m_atomUtf8String = internAtom("UTF8_STRING");
  m_atomWmName = internAtom("WM_NAME");
  m_atomNetClientList = internAtom("_NET_CLIENT_LIST");

  qInfo() << "X11Capture initialized - SHM:" << m_useSHM;
  return true;
}

void X11Capture::cleanup() {
  if (m_connection) {
    xcb_disconnect(m_connection);
    m_connection = nullptr;
  }
  m_screen = nullptr;
}

bool X11Capture::checkShmExtension() {
  xcb_shm_query_version_cookie_t cookie = xcb_shm_query_version(m_connection);
  xcb_shm_query_version_reply_t *reply =
      xcb_shm_query_version_reply(m_connection, cookie, nullptr);

  if (reply) {
    free(reply);
    return true;
  }
  return false;
}

xcb_window_t X11Capture::rootWindow() const {
  return m_screen ? m_screen->root : XCB_WINDOW_NONE;
}

QRect X11Capture::screenGeometry() const {
  if (!m_screen)
    return QRect();
  return QRect(0, 0, m_screen->width_in_pixels, m_screen->height_in_pixels);
}

QPixmap X11Capture::captureRect(xcb_drawable_t drawable, const QRect &rect) {
  if (!isReady() || rect.isEmpty()) {
    return QPixmap();
  }

  // Get image from X server
  xcb_get_image_cookie_t cookie =
      xcb_get_image(m_connection, XCB_IMAGE_FORMAT_Z_PIXMAP, drawable, rect.x(),
                    rect.y(), rect.width(), rect.height(),
                    ~0 // All planes
      );

  xcb_get_image_reply_t *reply =
      xcb_get_image_reply(m_connection, cookie, nullptr);

  if (!reply) {
    qWarning() << "Failed to get image from X server";
    return QPixmap();
  }

  // Get image data
  uint8_t *data = xcb_get_image_data(reply);
  int length = xcb_get_image_data_length(reply);

  // Determine depth and create QImage
  QImage::Format format;
  int bytesPerPixel;

  switch (reply->depth) {
  case 32:
    format = QImage::Format_ARGB32;
    bytesPerPixel = 4;
    break;
  case 24:
    format = QImage::Format_RGB32;
    bytesPerPixel = 4; // X11 pads to 32 bits
    break;
  default:
    qWarning() << "Unsupported depth:" << reply->depth;
    free(reply);
    return QPixmap();
  }

  // Create QImage from raw data (copy the data since reply will be freed)
  QImage image(rect.width(), rect.height(), format);

  int rowBytes = rect.width() * bytesPerPixel;
  for (int y = 0; y < rect.height(); ++y) {
    memcpy(image.scanLine(y), data + y * rowBytes, rowBytes);
  }

  free(reply);

  // X11 uses BGRA, Qt expects ARGB/RGBA
  // Swap red and blue channels
  image = image.rgbSwapped();

  return QPixmap::fromImage(std::move(image));
}

QPixmap X11Capture::captureFullscreen() {
  if (!isReady())
    return QPixmap();
  return captureRect(rootWindow(), screenGeometry());
}

QPixmap X11Capture::captureMonitor(int monitorIndex) {
  auto monitors = getMonitors();
  if (monitorIndex < 0 ||
      static_cast<size_t>(monitorIndex) >= monitors.size()) {
    return captureFullscreen(); // Fallback to fullscreen
  }
  return captureRect(rootWindow(), monitors[monitorIndex].geometry);
}

QPixmap X11Capture::captureRegion(const QRect &region) {
  if (!isReady())
    return QPixmap();

  // Clamp to screen bounds
  QRect screenRect = screenGeometry();
  QRect clampedRegion = region.intersected(screenRect);

  if (clampedRegion.isEmpty()) {
    return QPixmap();
  }

  return captureRect(rootWindow(), clampedRegion);
}

QPixmap X11Capture::captureWindow(uint32_t windowId) {
  if (!isReady())
    return QPixmap();

  // Get window geometry
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(m_connection, windowId);
  xcb_get_geometry_reply_t *geom =
      xcb_get_geometry_reply(m_connection, cookie, nullptr);

  if (!geom) {
    return QPixmap();
  }

  QRect rect(0, 0, geom->width, geom->height);
  free(geom);

  return captureRect(windowId, rect);
}

QPixmap X11Capture::captureActiveWindow() {
  xcb_window_t active = getActiveWindow();
  if (active == XCB_WINDOW_NONE) {
    return QPixmap();
  }
  return captureWindow(active);
}

xcb_window_t X11Capture::getActiveWindow() const {
  if (!isReady() || m_atomNetActiveWindow == XCB_ATOM_NONE) {
    return XCB_WINDOW_NONE;
  }

  xcb_get_property_cookie_t cookie =
      xcb_get_property(m_connection, 0, rootWindow(), m_atomNetActiveWindow,
                       XCB_ATOM_WINDOW, 0, 1);

  xcb_get_property_reply_t *reply =
      xcb_get_property_reply(m_connection, cookie, nullptr);

  if (!reply || xcb_get_property_value_length(reply) == 0) {
    if (reply)
      free(reply);
    return XCB_WINDOW_NONE;
  }

  xcb_window_t *window =
      static_cast<xcb_window_t *>(xcb_get_property_value(reply));
  xcb_window_t result = *window;
  free(reply);

  return result;
}

QString X11Capture::getWindowTitle(xcb_window_t window) const {
  if (!isReady())
    return QString();

  // Try _NET_WM_NAME first (UTF-8)
  if (m_atomNetWmName != XCB_ATOM_NONE) {
    xcb_get_property_cookie_t cookie = xcb_get_property(
        m_connection, 0, window, m_atomNetWmName, m_atomUtf8String, 0, 256);

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(m_connection, cookie, nullptr);

    if (reply && xcb_get_property_value_length(reply) > 0) {
      QString title =
          QString::fromUtf8(static_cast<char *>(xcb_get_property_value(reply)),
                            xcb_get_property_value_length(reply));
      free(reply);
      return title;
    }
    if (reply)
      free(reply);
  }

  // Fallback to WM_NAME (Latin-1)
  xcb_get_property_cookie_t cookie = xcb_get_property(
      m_connection, 0, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 256);

  xcb_get_property_reply_t *reply =
      xcb_get_property_reply(m_connection, cookie, nullptr);

  if (reply && xcb_get_property_value_length(reply) > 0) {
    QString title =
        QString::fromLatin1(static_cast<char *>(xcb_get_property_value(reply)),
                            xcb_get_property_value_length(reply));
    free(reply);
    return title;
  }
  if (reply)
    free(reply);

  return QString();
}

std::vector<MonitorInfo> X11Capture::getMonitors() {
  std::vector<MonitorInfo> monitors;

  if (!isReady())
    return monitors;

  // For now, return single monitor (TODO: use RANDR extension for
  // multi-monitor)
  MonitorInfo primary;
  primary.index = 0;
  primary.name = "Primary";
  primary.geometry = screenGeometry();
  primary.isPrimary = true;
  monitors.push_back(primary);

  return monitors;
}

std::vector<WindowInfo> X11Capture::getWindows() {
  std::vector<WindowInfo> windows;

  if (!isReady() || m_atomNetClientList == XCB_ATOM_NONE) {
    return windows;
  }

  // Get _NET_CLIENT_LIST
  xcb_get_property_cookie_t cookie =
      xcb_get_property(m_connection, 0, rootWindow(), m_atomNetClientList,
                       XCB_ATOM_WINDOW, 0, 1024);

  xcb_get_property_reply_t *reply =
      xcb_get_property_reply(m_connection, cookie, nullptr);

  if (!reply)
    return windows;

  int count = xcb_get_property_value_length(reply) / sizeof(xcb_window_t);
  xcb_window_t *windowList =
      static_cast<xcb_window_t *>(xcb_get_property_value(reply));

  for (int i = 0; i < count; ++i) {
    xcb_window_t wid = windowList[i];

    // Get window geometry
    xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry(m_connection, wid);
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(m_connection, geomCookie, nullptr);

    if (!geom)
      continue;

    WindowInfo info;
    info.id = wid;
    info.title = getWindowTitle(wid);
    info.geometry = QRect(geom->x, geom->y, geom->width, geom->height);
    info.isMinimized = false; // TODO: Check _NET_WM_STATE

    windows.push_back(info);
    free(geom);
  }

  free(reply);
  return windows;
}

} // namespace ShareY
