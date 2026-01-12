//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_BOARDVIEW_H
#define TIKTAKTOE_BOARDVIEW_H
#pragma once

#include <QGraphicsView>

class BoardView : public QGraphicsView {
    Q_OBJECT
public:
    explicit BoardView(QWidget* parent = nullptr);

    void setCellSize(int px);
    int cellSize() const { return cellSize_; }

    // finite bounds used for click validation; if width/height == 0 => infinite
    void setFiniteBounds(int width, int height);

    void resetViewToRect(const QRectF& rect);

    signals:
        void cellClicked(int x, int y);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int cellSize_ = 40;
    bool finite_ = true;
    int boardWidth_ = 0;
    int boardHeight_ = 0;

    bool panning_ = false;
    QPoint panStart_;
};

#endif //TIKTAKTOE_BOARDVIEW_H