#include "RegionSelector.hpp"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

namespace ShareY {

RegionSelector::RegionSelector(QWidget *parent)
    : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {

  setMouseTracking(true);
  setCursor(Qt::CrossCursor);
  setAttribute(Qt::WA_TranslucentBackground);
}

void RegionSelector::start(const QPixmap &background) {
  m_background = background;
  m_selecting = false;
  m_startPoint = QPoint();
  m_endPoint = QPoint();

  // Get primary screen geometry
  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen) {
    setGeometry(screen->geometry());
  }

  showFullScreen();
  activateWindow();
  raise();
  setFocus();
}

QRect RegionSelector::normalizedRect() const {
  return QRect(m_startPoint, m_endPoint).normalized();
}

void RegionSelector::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this);

  // Draw background
  painter.drawPixmap(0, 0, m_background);

  // Dark overlay
  painter.fillRect(rect(), QColor(0, 0, 0, 100));

  if (m_selecting || (!m_startPoint.isNull() && !m_endPoint.isNull())) {
    QRect selection = normalizedRect();

    if (!selection.isEmpty()) {
      // Clear selection area (show original image)
      painter.setCompositionMode(QPainter::CompositionMode_Clear);
      painter.fillRect(selection, Qt::transparent);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

      // Draw original image in selection
      painter.drawPixmap(selection, m_background, selection);

      // Selection border
      painter.setPen(QPen(QColor("#89b4fa"), 2, Qt::SolidLine));
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(selection);

      // Corner handles
      int handleSize = 8;
      painter.setBrush(QColor("#89b4fa"));
      painter.setPen(Qt::NoPen);

      QRect handles[] = {
          QRect(selection.left() - handleSize / 2,
                selection.top() - handleSize / 2, handleSize, handleSize),
          QRect(selection.right() - handleSize / 2,
                selection.top() - handleSize / 2, handleSize, handleSize),
          QRect(selection.left() - handleSize / 2,
                selection.bottom() - handleSize / 2, handleSize, handleSize),
          QRect(selection.right() - handleSize / 2,
                selection.bottom() - handleSize / 2, handleSize, handleSize),
      };

      for (const QRect &handle : handles) {
        painter.drawRect(handle);
      }

      // Size label
      QString sizeText =
          QString("%1 × %2").arg(selection.width()).arg(selection.height());

      QFont font("Sans", 11, QFont::Bold);
      painter.setFont(font);

      QFontMetrics fm(font);
      QRect textRect = fm.boundingRect(sizeText);
      textRect.adjust(-10, -5, 10, 5);

      int labelX = selection.center().x() - textRect.width() / 2;
      int labelY = selection.bottom() + 10;

      // Keep label on screen
      if (labelY + textRect.height() > height()) {
        labelY = selection.top() - textRect.height() - 10;
      }

      textRect.moveTo(labelX, labelY);

      painter.setBrush(QColor(0, 0, 0, 180));
      painter.setPen(Qt::NoPen);
      painter.drawRoundedRect(textRect, 5, 5);

      painter.setPen(QColor("#cdd6f4"));
      painter.drawText(textRect, Qt::AlignCenter, sizeText);
    }
  }

  // Instructions
  QString instructions =
      "Click and drag to select region. Press Escape to cancel.";
  QFont font("Sans", 12);
  painter.setFont(font);

  QFontMetrics fm(font);
  QRect textRect = fm.boundingRect(instructions);
  textRect.adjust(-20, -10, 20, 10);
  textRect.moveTo((width() - textRect.width()) / 2, 30);

  painter.setBrush(QColor(0, 0, 0, 180));
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(textRect, 8, 8);

  painter.setPen(QColor("#cdd6f4"));
  painter.drawText(textRect, Qt::AlignCenter, instructions);
}

void RegionSelector::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_selecting = true;
    m_startPoint = event->pos();
    m_endPoint = event->pos();
    update();
  }
}

void RegionSelector::mouseMoveEvent(QMouseEvent *event) {
  if (m_selecting) {
    m_endPoint = event->pos();
    update();
  }
}

void RegionSelector::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton && m_selecting) {
    m_selecting = false;
    m_endPoint = event->pos();

    QRect selection = normalizedRect();

    hide();

    if (selection.width() > 5 && selection.height() > 5) {
      emit regionSelected(selection);
    } else {
      emit cancelled();
    }
  }
}

void RegionSelector::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    hide();
    emit cancelled();
  }
}

} // namespace ShareY
