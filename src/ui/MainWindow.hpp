#pragma once

/**
 * Main Application Window
 *
 * The dashboard that displays captured screenshots as thumbnails.
 * Provides actions like copy, save, delete, and edit.
 */

#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <memory>

#include "capture/CaptureEngine.hpp"
#include "core/ScreenshotBuffer.hpp"

namespace ShareY {

class ThumbnailWidget;
class RegionSelector;

/**
 * Main window with screenshot dashboard
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  /**
   * Capture entire screen and add to buffer
   */
  void captureFullscreen();

  /**
   * Start region selection for capture
   */
  void captureRegion();

  /**
   * Capture a specific window
   */
  void captureWindow();

  /**
   * Capture the currently active window
   */
  void captureActiveWindow();

protected:
  void closeEvent(QCloseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  /**
   * Handle screenshot added to buffer
   */
  void onScreenshotAdded(const QUuid &id);

  /**
   * Handle screenshot removed from buffer
   */
  void onScreenshotRemoved(const QUuid &id);

  /**
   * Handle buffer cleared
   */
  void onBufferCleared();

  /**
   * Handle thumbnail click
   */
  void onThumbnailClicked(const QUuid &id);

  /**
   * Handle thumbnail double-click (copy to clipboard)
   */
  void onThumbnailDoubleClicked(const QUuid &id);

  /**
   * Handle region selection completed
   */
  void onRegionSelected(const QRect &region);

  /**
   * Copy screenshot to clipboard
   */
  void copyToClipboard(const QUuid &id);

  /**
   * Save screenshot to file
   */
  void saveToFile(const QUuid &id);

  /**
   * Delete screenshot from buffer
   */
  void deleteScreenshot(const QUuid &id);

  /**
   * Clear all screenshots
   */
  void clearAll();

private:
  void setupUI();
  void setupMenuBar();
  void setupToolBar();
  void setupStatusBar();
  void updateStatusBar();
  void refreshThumbnails();

  // Core components
  std::unique_ptr<CaptureEngine> m_captureEngine;
  ScreenshotBuffer m_buffer;

  // UI components
  QScrollArea *m_scrollArea{nullptr};
  QWidget *m_thumbnailContainer{nullptr};
  QLabel *m_statusLabel{nullptr};
  QLabel *m_memoryLabel{nullptr};
  RegionSelector *m_regionSelector{nullptr};

  // Thumbnail widgets mapped by screenshot ID
  std::map<QUuid, ThumbnailWidget *> m_thumbnails;
};

} // namespace ShareY
