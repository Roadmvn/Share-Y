#include "MainWindow.hpp"
#include "RegionSelector.hpp"
#include "ThumbnailWidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QFileDialog>
#include "FlowLayout.hpp"
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

namespace ShareY {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

  // Create capture engine
  m_captureEngine = CaptureEngine::create();
  if (!m_captureEngine || !m_captureEngine->isReady()) {
    qWarning() << "Failed to initialize capture engine";
  }

  setupUI();
  setupMenuBar();
  setupToolBar();
  setupStatusBar();

  // Connect buffer signals
  connect(&m_buffer, &ScreenshotBuffer::screenshotAdded, this,
          &MainWindow::onScreenshotAdded);
  connect(&m_buffer, &ScreenshotBuffer::screenshotRemoved, this,
          &MainWindow::onScreenshotRemoved);
  connect(&m_buffer, &ScreenshotBuffer::bufferCleared, this,
          &MainWindow::onBufferCleared);
  connect(&m_buffer, &ScreenshotBuffer::memoryUsageChanged, this,
          [this](size_t, size_t) { updateStatusBar(); });

  setWindowTitle("ShareY - Screenshot Manager");
  setMinimumSize(800, 600);
  resize(1200, 800);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
  // Central widget with scroll area
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Container for thumbnails
  m_thumbnailContainer = new QWidget();
  m_thumbnailContainer->setObjectName("thumbnailContainer");

  // Flow layout for thumbnails (wraps automatically)
  auto *layout = new FlowLayout(m_thumbnailContainer);
  layout->setSpacing(10);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Use a grid-like wrap behavior
  m_thumbnailContainer->setLayout(layout);
  m_scrollArea->setWidget(m_thumbnailContainer);

  setCentralWidget(m_scrollArea);

  // Apply stylesheet
  setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e2e;
        }
        #thumbnailContainer {
            background-color: #1e1e2e;
        }
        QScrollArea {
            border: none;
            background-color: #1e1e2e;
        }
        QToolBar {
            background-color: #313244;
            border: none;
            spacing: 5px;
            padding: 5px;
        }
        QToolBar QToolButton {
            background-color: #45475a;
            border: none;
            border-radius: 5px;
            padding: 8px 15px;
            color: #cdd6f4;
            font-weight: bold;
        }
        QToolBar QToolButton:hover {
            background-color: #585b70;
        }
        QToolBar QToolButton:pressed {
            background-color: #6c7086;
        }
        QStatusBar {
            background-color: #313244;
            color: #a6adc8;
        }
        QMenuBar {
            background-color: #313244;
            color: #cdd6f4;
        }
        QMenuBar::item:selected {
            background-color: #45475a;
        }
        QMenu {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
        }
        QMenu::item:selected {
            background-color: #45475a;
        }
    )");
}

void MainWindow::setupMenuBar() {
  QMenuBar *menuBar = this->menuBar();

  // File menu
  QMenu *fileMenu = menuBar->addMenu("&File");

  QAction *saveAllAction = fileMenu->addAction("Save All...");
  connect(saveAllAction, &QAction::triggered, this, [this]() {
    // TODO: Implement save all
  });

  fileMenu->addSeparator();

  QAction *clearAction = fileMenu->addAction("Clear All");
  clearAction->setShortcut(QKeySequence("Ctrl+Shift+Delete"));
  connect(clearAction, &QAction::triggered, this, &MainWindow::clearAll);

  fileMenu->addSeparator();

  QAction *quitAction = fileMenu->addAction("Quit");
  quitAction->setShortcut(QKeySequence::Quit);
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

  // Capture menu
  QMenu *captureMenu = menuBar->addMenu("&Capture");

  QAction *fullscreenAction = captureMenu->addAction("Fullscreen");
  fullscreenAction->setShortcut(QKeySequence("Print"));
  connect(fullscreenAction, &QAction::triggered, this,
          &MainWindow::captureFullscreen);

  QAction *regionAction = captureMenu->addAction("Region...");
  regionAction->setShortcut(QKeySequence("Ctrl+Print"));
  connect(regionAction, &QAction::triggered, this, &MainWindow::captureRegion);

  QAction *windowAction = captureMenu->addAction("Window...");
  windowAction->setShortcut(QKeySequence("Alt+Print"));
  connect(windowAction, &QAction::triggered, this, &MainWindow::captureWindow);

  QAction *activeWindowAction = captureMenu->addAction("Active Window");
  activeWindowAction->setShortcut(QKeySequence("Shift+Print"));
  connect(activeWindowAction, &QAction::triggered, this,
          &MainWindow::captureActiveWindow);

  // Help menu
  QMenu *helpMenu = menuBar->addMenu("&Help");

  QAction *aboutAction = helpMenu->addAction("About ShareY");
  connect(aboutAction, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, "About ShareY",
                       "<h2>ShareY</h2>"
                       "<p>Version 0.1.0</p>"
                       "<p>A lightweight screenshot manager for Linux.</p>"
                       "<p>Inspired by ShareX for Windows.</p>");
  });
}

void MainWindow::setupToolBar() {
  QToolBar *toolBar = addToolBar("Main");
  toolBar->setMovable(false);
  toolBar->setIconSize(QSize(24, 24));

  QAction *fullscreenAction = toolBar->addAction("Fullscreen");
  connect(fullscreenAction, &QAction::triggered, this,
          &MainWindow::captureFullscreen);

  QAction *regionAction = toolBar->addAction("Region");
  connect(regionAction, &QAction::triggered, this, &MainWindow::captureRegion);

  QAction *windowAction = toolBar->addAction("Window");
  connect(windowAction, &QAction::triggered, this, &MainWindow::captureWindow);

  toolBar->addSeparator();

  QAction *clearAction = toolBar->addAction("Clear All");
  connect(clearAction, &QAction::triggered, this, &MainWindow::clearAll);
}

void MainWindow::setupStatusBar() {
  QStatusBar *status = statusBar();

  m_statusLabel = new QLabel("Ready");
  status->addWidget(m_statusLabel, 1);

  m_memoryLabel = new QLabel();
  status->addPermanentWidget(m_memoryLabel);

  updateStatusBar();
}

void MainWindow::updateStatusBar() {
  size_t count = m_buffer.count();
  size_t memoryBytes = m_buffer.totalMemoryUsage();
  size_t limitBytes = m_buffer.memoryLimit();

  m_statusLabel->setText(QString("%1 screenshot(s)").arg(count));

  double memoryMB = static_cast<double>(memoryBytes) / (1024 * 1024);
  double limitMB = static_cast<double>(limitBytes) / (1024 * 1024);
  m_memoryLabel->setText(
      QString("Memory: %.1f / %.0f MB").arg(memoryMB).arg(limitMB));
}

void MainWindow::captureFullscreen() {
  if (!m_captureEngine || !m_captureEngine->isReady()) {
    QMessageBox::warning(this, "Error", "Capture engine not available");
    return;
  }

  // Hide window briefly to avoid capturing it
  hide();
  QTimer::singleShot(100, this, [this]() {
    QPixmap pixmap = m_captureEngine->captureFullscreen();
    show();

    if (!pixmap.isNull()) {
      Screenshot screenshot =
          Screenshot::create(std::move(pixmap), CaptureType::Fullscreen);
      m_buffer.add(std::move(screenshot));
    } else {
      QMessageBox::warning(this, "Error", "Failed to capture screen");
    }
  });
}

void MainWindow::captureRegion() {
  if (!m_captureEngine || !m_captureEngine->isReady()) {
    QMessageBox::warning(this, "Error", "Capture engine not available");
    return;
  }

  // Create region selector if needed
  if (!m_regionSelector) {
    m_regionSelector = new RegionSelector();
    connect(m_regionSelector, &RegionSelector::regionSelected, this,
            &MainWindow::onRegionSelected);
    connect(m_regionSelector, &RegionSelector::cancelled, this,
            [this]() { show(); });
  }

  hide();

  // Capture fullscreen for overlay
  QPixmap background = m_captureEngine->captureFullscreen();
  m_regionSelector->start(background);
}

void MainWindow::onRegionSelected(const QRect &region) {
  if (region.isEmpty()) {
    show();
    return;
  }

  QPixmap pixmap = m_captureEngine->captureRegion(region);
  show();

  if (!pixmap.isNull()) {
    Screenshot screenshot =
        Screenshot::create(std::move(pixmap), CaptureType::Region);
    m_buffer.add(std::move(screenshot));
  }
}

void MainWindow::captureWindow() {
  // TODO: Show window picker dialog
  captureActiveWindow();
}

void MainWindow::captureActiveWindow() {
  if (!m_captureEngine || !m_captureEngine->isReady()) {
    QMessageBox::warning(this, "Error", "Capture engine not available");
    return;
  }

  hide();
  QTimer::singleShot(200, this, [this]() {
    QPixmap pixmap = m_captureEngine->captureActiveWindow();
    show();

    if (!pixmap.isNull()) {
      Screenshot screenshot =
          Screenshot::create(std::move(pixmap), CaptureType::ActiveWindow);
      m_buffer.add(std::move(screenshot));
    }
  });
}

void MainWindow::onScreenshotAdded(const QUuid &id) {
  const Screenshot *screenshot = m_buffer.get(id);
  if (!screenshot)
    return;

  // Create thumbnail widget
  auto *thumbnail = new ThumbnailWidget(*screenshot, m_thumbnailContainer);

  connect(thumbnail, &ThumbnailWidget::clicked, this,
          &MainWindow::onThumbnailClicked);
  connect(thumbnail, &ThumbnailWidget::doubleClicked, this,
          &MainWindow::onThumbnailDoubleClicked);
  connect(thumbnail, &ThumbnailWidget::copyRequested, this,
          &MainWindow::copyToClipboard);
  connect(thumbnail, &ThumbnailWidget::saveRequested, this,
          &MainWindow::saveToFile);
  connect(thumbnail, &ThumbnailWidget::deleteRequested, this,
          &MainWindow::deleteScreenshot);

  // Add to layout at the beginning
  auto *layout = m_thumbnailContainer->layout();
  static_cast<QHBoxLayout *>(layout)->insertWidget(0, thumbnail);

  m_thumbnails[id] = thumbnail;
  updateStatusBar();
}

void MainWindow::onScreenshotRemoved(const QUuid &id) {
  auto it = m_thumbnails.find(id);
  if (it != m_thumbnails.end()) {
    it->second->deleteLater();
    m_thumbnails.erase(it);
  }
  updateStatusBar();
}

void MainWindow::onBufferCleared() {
  for (auto &[id, widget] : m_thumbnails) {
    widget->deleteLater();
  }
  m_thumbnails.clear();
  updateStatusBar();
}

void MainWindow::onThumbnailClicked(const QUuid &id) {
  // Select thumbnail (future: multi-select)
  for (auto &[thumbId, widget] : m_thumbnails) {
    widget->setSelected(thumbId == id);
  }
}

void MainWindow::onThumbnailDoubleClicked(const QUuid &id) {
  copyToClipboard(id);
}

void MainWindow::copyToClipboard(const QUuid &id) {
  const Screenshot *screenshot = m_buffer.get(id);
  if (!screenshot)
    return;

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setPixmap(screenshot->pixmap);

  m_statusLabel->setText("Copied to clipboard!");
  QTimer::singleShot(2000, this, [this]() { updateStatusBar(); });
}

void MainWindow::saveToFile(const QUuid &id) {
  const Screenshot *screenshot = m_buffer.get(id);
  if (!screenshot)
    return;

  QString defaultName =
      QString("screenshot_%1.png")
          .arg(screenshot->timestamp.toString("yyyyMMdd_hhmmss"));

  QString filePath = QFileDialog::getSaveFileName(
      this, "Save Screenshot", defaultName,
      "PNG Image (*.png);;JPEG Image (*.jpg);;All Files (*)");

  if (!filePath.isEmpty()) {
    if (screenshot->pixmap.save(filePath)) {
      m_statusLabel->setText(QString("Saved to %1").arg(filePath));
    } else {
      QMessageBox::warning(this, "Error", "Failed to save screenshot");
    }
  }
}

void MainWindow::deleteScreenshot(const QUuid &id) { m_buffer.remove(id); }

void MainWindow::clearAll() {
  if (m_buffer.isEmpty())
    return;

  QMessageBox::StandardButton reply = QMessageBox::question(
      this, "Clear All",
      QString("Delete all %1 screenshots from memory?").arg(m_buffer.count()),
      QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    m_buffer.clear();
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Hide to tray instead of closing
  hide();
  event->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    hide();
    return;
  }
  QMainWindow::keyPressEvent(event);
}

void MainWindow::refreshThumbnails() {
  // Clear existing
  for (auto &[id, widget] : m_thumbnails) {
    widget->deleteLater();
  }
  m_thumbnails.clear();

  // Recreate from buffer
  for (const QUuid &id : m_buffer.allIds()) {
    onScreenshotAdded(id);
  }
}

} // namespace ShareY
