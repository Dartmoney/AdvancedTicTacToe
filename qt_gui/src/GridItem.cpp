#include "GridItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <cmath>

GridItem::GridItem(const QRectF& rect, int cellSizePx, bool drawBorder)
    : rect_(rect), cellSize_(std::max(2, cellSizePx)), drawBorder_(drawBorder) {
    setZValue(0);
}

void GridItem::setRect(const QRectF& r) {
    prepareGeometryChange();
    rect_ = r;
}

void GridItem::setCellSize(int px) {
    cellSize_ = std::max(2, px);
}

void GridItem::setDrawBorder(bool b) {
    drawBorder_ = b;
}

void GridItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    QRectF exposed = option->exposedRect.intersected(rect_);
    if (!exposed.isValid() || exposed.isEmpty()) return;

    QPen gridPen(Qt::lightGray);
    gridPen.setWidthF(0.0);
    painter->setPen(gridPen);

    const qreal left = exposed.left();
    const qreal right = exposed.right();
    const qreal top = exposed.top();
    const qreal bottom = exposed.bottom();

    const int cs = cellSize_;

    const qreal startX = std::floor(left / cs) * cs;
    const qreal endX = std::ceil(right / cs) * cs;
    const qreal startY = std::floor(top / cs) * cs;
    const qreal endY = std::ceil(bottom / cs) * cs;

    for (qreal x = startX; x <= endX; x += cs) {
        painter->drawLine(QPointF(x, top), QPointF(x, bottom));
    }
    for (qreal y = startY; y <= endY; y += cs) {
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    }

    if (drawBorder_) {
        QPen borderPen(Qt::gray);
        borderPen.setWidthF(0.0);
        painter->setPen(borderPen);
        painter->drawRect(rect_);
    }
}
