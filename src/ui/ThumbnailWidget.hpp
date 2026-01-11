#pragma once

/**
 * Thumbnail Widget
 *
 * Displays a single screenshot as a clickable thumbnail.
 * Provides context menu for copy, save, delete, edit.
 */

#include <QLabel>
#include <QUuid>
#include <QWidget>

#include "core/Screenshot.hpp"

namespace ShareY {

/**
 * Thumbnail widget for a single screenshot
 */
class ThumbnailWidget : public QWidget {
  Q_OBJECT

public:
  explicit ThumbnailWidget(const Screenshot &screenshot,
                           QWidget *parent = nullptr);
  ~ThumbnailWidget() override = default;

  /**
   * Get the screenshot ID this thumbnail represents
   */
  QUuid screenshotId() const { return m_id; }

  /**
   * Set selection state
   */
  void setSelected(bool selected);

  /**
   * Check if selected
   */
  bool isSelected() const { return m_selected; }

signals:
  void clicked(const QUuid &id);
  void doubleClicked(const QUuid &id);
  void copyRequested(const QUuid &id);
  void saveRequested(const QUuid &id);
  void deleteRequested(const QUuid &id);
  void editRequested(const QUuid &id);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

private:
  void setupUI(const Screenshot &screenshot);

  QUuid m_id;
  QPixmap m_thumbnail;
  QString m_sizeText;
  QString m_timeText;

  bool m_selected{false};
  bool m_hovered{false};

  static constexpr int THUMBNAIL_SIZE = 200;
  static constexpr int PADDING = 10;
  static constexpr int INFO_HEIGHT = 40;
};

} // namespace ShareY
