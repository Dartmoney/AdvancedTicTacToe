#ifndef TIKTAKTOE_MAINWINDOW_H
#define TIKTAKTOE_MAINWINDOW_H
#pragma once

#include <QMainWindow>
#include <QGraphicsScene>

#include <unordered_map>

#include <engine/AI.h>
#include <engine/GameState.h>

#include "AppConfig.h"

class BoardView;
class GridItem;
class SettingsPanel;
class QGraphicsRectItem;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const AppConfig& cfg, QWidget* parent = nullptr);

private slots:
    void onCellClicked(int x, int y);
    void onNewGameRequested();
    void onUndoRequested();
    void onRedoRequested();
    void onNextTurnRequested();
    void onResetViewRequested();

private:
    void startNewGame(const engine::RuleSet& rules);
    void rebuildScene();
    void syncSceneWithBoard();
    void updateUi();
    bool isAiVsAiModeActive() const;
    void ensureAiMoveIfNeeded();
    void performAiMove(engine::Player aiPlayer);

    void updateSceneRectForTopology();
    QRectF boardSceneRect() const;

private:
    AppConfig cfg_;

    engine::GameState game_;
    engine::SimpleAI ai_;

    bool aiEnabled_ = false;
    engine::Player aiPlayer_ = engine::Player::O;
    int aiRadius_ = 2;

    QGraphicsScene* scene_ = nullptr;
    BoardView* view_ = nullptr;
    SettingsPanel* settings_ = nullptr;

    GridItem* grid_ = nullptr;
    QGraphicsRectItem* lastMoveHighlight_ = nullptr;

    int cellSize_ = 40;

    std::unordered_map<engine::Coord, QGraphicsItem*, engine::CoordHash> markItems_;
};

#endif