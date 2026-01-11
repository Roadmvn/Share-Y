#pragma once

/**
 * Portal-based screen capture for Wayland
 * Uses org.freedesktop.portal.Screenshot via DBus
 */

#include "CaptureEngine.hpp"

namespace ShareY {

class PortalCapture : public CaptureEngine {
public:
    PortalCapture();
    ~PortalCapture() override;

    bool initialize() override;
    void cleanup() override;
    bool isReady() const override { return m_ready; }

    QPixmap captureFullscreen() override;
    QPixmap captureMonitor(int monitorIndex) override;
    QPixmap captureRegion(const QRect &region) override;
    QPixmap captureWindow(uint32_t windowId) override;
    QPixmap captureActiveWindow() override;

    std::vector<MonitorInfo> getMonitors() override;
    std::vector<WindowInfo> getWindows() override;

    QString engineName() const override { return "Portal (Wayland)"; }

private:
    QPixmap captureViaPortal();
    QPixmap captureViaGnomeShell();
    QPixmap captureViaGnomeScreenshot(bool interactive = false);
    QPixmap captureViaGrimshot();
    QPixmap captureViaSpectacle(bool fullscreen = true);
    
    bool m_ready{false};
    bool m_hasGnomeScreenshot{false};
    bool m_hasGrim{false};
};

} // namespace ShareY
