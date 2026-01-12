#include "BoardView.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <cmath>

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void BoardView::setCellSize(int px) {
    cellSize_ = std::max(2, px);
}

void BoardView::setFiniteBounds(int width, int height) {
    if (width > 0 && height > 0) {
        finite_ = true;
        boardWidth_ = width;
        boardHeight_ = height;
    } else {
        finite_ = false;
        boardWidth_ = 0;
        boardHeight_ = 0;
    }
}

void BoardView::resetViewToRect(const QRectF& rect) {
    resetTransform();
    if (rect.isValid() && !rect.isEmpty()) {
        fitInView(rect, Qt::KeepAspectRatio);
    }
}

void BoardView::wheelEvent(QWheelEvent* event) {
    constexpr qreal factor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(factor, factor);
    } else {
        scale(1.0 / factor, 1.0 / factor);
    }
    event->accept();
}

void BoardView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        panning_ = true;
        panStart_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && !panning_) {
        const QPointF sp = mapToScene(event->pos());
        const int x = static_cast<int>(std::floor(sp.x() / cellSize_));
        const int y = static_cast<int>(std::floor(sp.y() / cellSize_));

        if (finite_) {
            if (x < 0 || y < 0 || x >= boardWidth_ || y >= boardHeight_) {
                event->accept();
                return;
            }
        }

        emit cellClicked(x, y);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void BoardView::mouseMoveEvent(QMouseEvent* event) {
    if (panning_) {
        const QPoint delta = event->pos() - panStart_;
        panStart_ = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void BoardView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && panning_) {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}
