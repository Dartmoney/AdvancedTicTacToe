#include "MarkItem.h"

#include <QColor>
#include <QPainter>

MarkItem::MarkItem(engine::Player p, const QRectF& cellRect)
    : player_(p), rect_(cellRect) {
    setZValue(2);
}

void MarkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Цвета:
    // X — красный, O — синий
    QColor color = Qt::black;
    if (player_ == engine::Player::X) color = QColor(220, 30, 30);
    if (player_ == engine::Player::O) color = QColor(30, 60, 220);

    QPen pen(color);
    pen.setWidthF(2.4);
    painter->setPen(pen);

    const qreal m = rect_.width() * 0.18; // margin
    const QRectF r = rect_.adjusted(m, m, -m, -m);

    if (player_ == engine::Player::X) {
        painter->drawLine(r.topLeft(), r.bottomRight());
        painter->drawLine(r.topRight(), r.bottomLeft());
    } else if (player_ == engine::Player::O) {
        painter->drawEllipse(r);
    }
}
