#include "PortalCapture.hpp"

#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QDateTime>
#include <QFile>
#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QUuid>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QElapsedTimer>

namespace ShareY {

PortalCapture::PortalCapture() = default;

PortalCapture::~PortalCapture() {
    cleanup();
}

bool PortalCapture::initialize() {
    // Check if xdg-desktop-portal is available
    QDBusInterface portalInterface(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Screenshot",
        QDBusConnection::sessionBus()
    );
    
    m_ready = portalInterface.isValid();
    
    if (m_ready) {
        qInfo() << "PortalCapture initialized with xdg-desktop-portal";
    } else {
        qWarning() << "PortalCapture: xdg-desktop-portal not available";
    }
    
    // Also check for gnome-screenshot as fallback
    QProcess check;
    check.start("which", {"gnome-screenshot"});
    check.waitForFinished(1000);
    m_hasGnomeScreenshot = (check.exitCode() == 0);
    
    check.start("which", {"grim"});
    check.waitForFinished(1000);
    m_hasGrim = (check.exitCode() == 0);
    
    m_ready = m_ready || m_hasGnomeScreenshot || m_hasGrim;
    
    return m_ready;
}

void PortalCapture::cleanup() {
    m_ready = false;
}

QPixmap PortalCapture::captureViaPortal() {
    qInfo() << "Attempting interactive portal screenshot...";
    
    // Generate a unique token
    QString token = QString("sharey_%1").arg(QRandomGenerator::global()->generate());
    QString senderName = QDBusConnection::sessionBus().baseService().mid(1).replace('.', '_');
    QString requestPath = QString("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(senderName)
        .arg(token);
    
    // Create options - set interactive to true for GNOME dialog
    QVariantMap options;
    options["handle_token"] = token;
    options["interactive"] = true;  // This shows the GNOME share dialog
    options["modal"] = true;
    
    // Call Screenshot method
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Screenshot",
        "Screenshot"
    );
    
    msg << QString("")  // parent_window
        << options;
    
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg, QDBus::Block, 5000);
    
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Portal Screenshot call failed:" << reply.errorMessage();
        return QPixmap();
    }
    
    // Get the request path from the reply
    if (reply.arguments().isEmpty()) {
        qWarning() << "Portal Screenshot: empty reply";
        return QPixmap();
    }
    
    QDBusObjectPath replyPath = reply.arguments().at(0).value<QDBusObjectPath>();
    qInfo() << "Request path:" << replyPath.path();
    
    // Now we need to poll for the response by checking the Response signal
    // Use a simpler approach: just wait and poll the portal for screenshot files
    qInfo() << "Waiting for user to complete screenshot selection...";
    
    // Monitor for new files in the screenshot directory
    QString screenshotDir = QDir::homePath() + "/Images/Captures d'écran";
    QDir dir(screenshotDir);
    
    // Get current files
    QStringList existingFiles = dir.entryList(QStringList() << "*.png", QDir::Files);
    
    // Wait for user interaction (poll for new file)
    QElapsedTimer timer;
    timer.start();
    
    QString newFile;
    while (timer.elapsed() < 60000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QThread::msleep(200);
        
        QStringList currentFiles = dir.entryList(QStringList() << "*.png", QDir::Files);
        
        // Find new file
        for (const QString& file : currentFiles) {
            if (!existingFiles.contains(file)) {
                newFile = screenshotDir + "/" + file;
                qInfo() << "New screenshot file detected:" << newFile;
                break;
            }
        }
        
        if (!newFile.isEmpty()) {
            break;
        }
    }
    
    if (newFile.isEmpty()) {
        qWarning() << "No new screenshot file detected (timeout or cancelled)";
        return QPixmap();
    }
    
    // Wait a moment for file to be fully written
    QThread::msleep(200);
    
    QPixmap pixmap(newFile);
    
    if (pixmap.isNull()) {
        qWarning() << "Failed to load screenshot from" << newFile;
    } else {
        qInfo() << "Screenshot loaded successfully:" << pixmap.size();
    }
    
    return pixmap;
}

QPixmap PortalCapture::captureViaGnomeShell() {
    return QPixmap();
}

QPixmap PortalCapture::captureViaGnomeScreenshot(bool interactive) {
    QString tempPath = QDir::tempPath() + "/sharey_capture_" + 
                       QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    
    QProcess proc;
    QStringList args;
    
    if (interactive) {
        args << "-a" << "-f" << tempPath;
    } else {
        args << "-f" << tempPath;
    }
    
    proc.start("gnome-screenshot", args);
    
    int timeout = interactive ? 60000 : 5000;
    if (!proc.waitForFinished(timeout)) {
        qWarning() << "gnome-screenshot timed out";
        return QPixmap();
    }
    
    if (proc.exitCode() != 0) {
        qWarning() << "gnome-screenshot failed:" << proc.readAllStandardError();
        return QPixmap();
    }
    
    QPixmap pixmap(tempPath);
    QFile::remove(tempPath);
    
    return pixmap;
}

QPixmap PortalCapture::captureViaGrimshot() {
    return QPixmap();
}

QPixmap PortalCapture::captureViaSpectacle(bool fullscreen) {
    Q_UNUSED(fullscreen);
    return QPixmap();
}

QPixmap PortalCapture::captureFullscreen() {
    if (!m_ready) return QPixmap();
    
    qInfo() << "PortalCapture: Capturing via interactive portal...";
    qInfo() << "A GNOME dialog will appear - please click 'Share' to capture the screen.";
    
    QPixmap result = captureViaPortal();
    
    if (!result.isNull()) {
        qInfo() << "Capture successful, returning pixmap of size:" << result.size();
        return result;
    }
    
    qWarning() << "Portal capture failed or was cancelled by user";
    return QPixmap();
}

QPixmap PortalCapture::captureMonitor(int monitorIndex) {
    Q_UNUSED(monitorIndex);
    return captureFullscreen();
}

QPixmap PortalCapture::captureRegion(const QRect &region) {
    Q_UNUSED(region);
    return captureFullscreen();
}

QPixmap PortalCapture::captureWindow(uint32_t windowId) {
    Q_UNUSED(windowId);
    return captureFullscreen();
}

QPixmap PortalCapture::captureActiveWindow() {
    return captureFullscreen();
}

std::vector<MonitorInfo> PortalCapture::getMonitors() {
    std::vector<MonitorInfo> monitors;
    
    QList<QScreen*> screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        MonitorInfo info;
        info.index = i;
        info.name = screen->name();
        info.geometry = screen->geometry();
        info.isPrimary = (screen == QGuiApplication::primaryScreen());
        monitors.push_back(info);
    }
    
    return monitors;
}

std::vector<WindowInfo> PortalCapture::getWindows() {
    return {};
}

} // namespace ShareY
