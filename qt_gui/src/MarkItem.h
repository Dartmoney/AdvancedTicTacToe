//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_MARKITEM_H
#define TIKTAKTOE_MARKITEM_H
#pragma once

#include <QGraphicsItem>
#include <QRectF>

#include <engine/Player.h>

class MarkItem : public QGraphicsItem {
public:
    MarkItem(engine::Player p, const QRectF& cellRect);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    engine::Player player_;
    QRectF rect_;
};

#endif //TIKTAKTOE_MARKITEM_H