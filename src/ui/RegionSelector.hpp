#pragma once

/**
 * Region Selector
 *
 * Fullscreen overlay that allows the user to select a rectangular
 * region for capture using click-and-drag.
 */

#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QWidget>

namespace ShareY {

/**
 * Fullscreen region selection overlay
 */
class RegionSelector : public QWidget {
  Q_OBJECT

public:
  explicit RegionSelector(QWidget *parent = nullptr);
  ~RegionSelector() override = default;

  /**
   * Start region selection with a background image
   * @param background The captured screen to show as background
   */
  void start(const QPixmap &background);

signals:
  /**
   * Emitted when a region is selected
   */
  void regionSelected(const QRect &region);

  /**
   * Emitted when selection is cancelled (Escape key)
   */
  void cancelled();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  QRect normalizedRect() const;

  QPixmap m_background;
  QPoint m_startPoint;
  QPoint m_endPoint;
  bool m_selecting{false};
};

} // namespace ShareY
