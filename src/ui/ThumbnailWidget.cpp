#include "ThumbnailWidget.hpp"

#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

namespace ShareY {

ThumbnailWidget::ThumbnailWidget(const Screenshot &screenshot, QWidget *parent)
    : QWidget(parent), m_id(screenshot.id) {
  setupUI(screenshot);
}

void ThumbnailWidget::setupUI(const Screenshot &screenshot) {
  // Generate thumbnail
  m_thumbnail = screenshot.thumbnail(THUMBNAIL_SIZE);

  // Store metadata
  m_sizeText = screenshot.sizeString();
  m_timeText = screenshot.timestamp.toString("hh:mm:ss");

  // Calculate widget size
  int width = THUMBNAIL_SIZE + PADDING * 2;
  int height = THUMBNAIL_SIZE + INFO_HEIGHT + PADDING * 2;

  setFixedSize(width, height);
  setCursor(Qt::PointingHandCursor);
  setMouseTracking(true);
}

void ThumbnailWidget::setSelected(bool selected) {
  if (m_selected != selected) {
    m_selected = selected;
    update();
  }
}

void ThumbnailWidget::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  // Background
  QColor bgColor = m_selected  ? QColor("#45475a")
                   : m_hovered ? QColor("#313244")
                               : QColor("#1e1e2e");

  painter.setBrush(bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(rect(), 10, 10);

  // Border
  if (m_selected) {
    painter.setPen(QPen(QColor("#89b4fa"), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
  } else if (m_hovered) {
    painter.setPen(QPen(QColor("#6c7086"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
  }

  // Thumbnail image
  int thumbX = (width() - m_thumbnail.width()) / 2;
  int thumbY = PADDING;

  // Draw shadow
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(0, 0, 0, 50));
  painter.drawRoundedRect(thumbX + 3, thumbY + 3, m_thumbnail.width(),
                          m_thumbnail.height(), 5, 5);

  // Draw thumbnail
  painter.drawPixmap(thumbX, thumbY, m_thumbnail);

  // Draw rounded corners mask
  painter.setPen(Qt::NoPen);
  painter.setBrush(bgColor);

  // Info text
  int textY = PADDING + THUMBNAIL_SIZE + 5;

  painter.setPen(QColor("#cdd6f4"));
  painter.setFont(QFont("Sans", 9, QFont::Bold));
  painter.drawText(PADDING, textY, width() - PADDING * 2, 20,
                   Qt::AlignLeft | Qt::AlignVCenter, m_sizeText);

  painter.setPen(QColor("#a6adc8"));
  painter.setFont(QFont("Sans", 8));
  painter.drawText(PADDING, textY, width() - PADDING * 2, 20,
                   Qt::AlignRight | Qt::AlignVCenter, m_timeText);
}

void ThumbnailWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    emit clicked(m_id);
  }
  QWidget::mousePressEvent(event);
}

void ThumbnailWidget::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    emit doubleClicked(m_id);
  }
  QWidget::mouseDoubleClickEvent(event);
}

void ThumbnailWidget::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu(this);

  // Style the menu
  menu.setStyleSheet(R"(
        QMenu {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 5px;
            padding: 5px;
        }
        QMenu::item {
            padding: 8px 25px;
            border-radius: 3px;
        }
        QMenu::item:selected {
            background-color: #45475a;
        }
        QMenu::separator {
            height: 1px;
            background: #45475a;
            margin: 5px 0;
        }
    )");

  QAction *copyAction = menu.addAction("📋 Copy to Clipboard");
  connect(copyAction, &QAction::triggered, this,
          [this]() { emit copyRequested(m_id); });

  QAction *saveAction = menu.addAction("💾 Save to File...");
  connect(saveAction, &QAction::triggered, this,
          [this]() { emit saveRequested(m_id); });

  menu.addSeparator();

  QAction *editAction = menu.addAction("✏️ Edit/Annotate");
  connect(editAction, &QAction::triggered, this,
          [this]() { emit editRequested(m_id); });

  menu.addSeparator();

  QAction *deleteAction = menu.addAction("🗑️ Delete");
  connect(deleteAction, &QAction::triggered, this,
          [this]() { emit deleteRequested(m_id); });

  menu.exec(event->globalPos());
}

void ThumbnailWidget::enterEvent(QEnterEvent * /*event*/) {
  m_hovered = true;
  update();
}

void ThumbnailWidget::leaveEvent(QEvent * /*event*/) {
  m_hovered = false;
  update();
}

} // namespace ShareY
