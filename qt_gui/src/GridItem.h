//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_GRIDITEM_H
#define TIKTAKTOE_GRIDITEM_H
#pragma once

#include <QGraphicsItem>
#include <QRectF>

class GridItem : public QGraphicsItem {
public:
    GridItem(const QRectF& rect, int cellSizePx, bool drawBorder);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setRect(const QRectF& r);
    void setCellSize(int px);
    void setDrawBorder(bool b);

private:
    QRectF rect_;
    int cellSize_ = 40;
    bool drawBorder_ = true;
};

#endif //TIKTAKTOE_GRIDITEM_H