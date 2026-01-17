#include "MainWindow.h"

#include "BoardView.h"
#include "GameOverDialog.h"
#include "GridItem.h"
#include "MarkItem.h"
#include "SettingsPanel.h"

#include <QDockWidget>
#include <QGraphicsRectItem>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>

MainWindow::MainWindow(const AppConfig& cfg, QWidget* parent)
    : QMainWindow(parent),
      cfg_(cfg),
      game_(),
      ai_(engine::SimpleAI::Settings{cfg.aiCandidateRadius, 600, 0}) {
    setWindowTitle("Advanced Tic-Tac-Toe (Qt6)");

    cellSize_ = cfg_.cellSizePx;

    scene_ = new QGraphicsScene(this);

    view_ = new BoardView(this);
    view_->setScene(scene_);
    view_->setCellSize(cellSize_);
    setCentralWidget(view_);

    settings_ = new SettingsPanel(this);
    auto* dock = new QDockWidget("Settings", this);
    dock->setWidget(settings_);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(view_, &BoardView::cellClicked, this, &MainWindow::onCellClicked);
    connect(settings_, &SettingsPanel::newGameRequested, this, &MainWindow::onNewGameRequested);
    connect(settings_, &SettingsPanel::undoRequested, this, &MainWindow::onUndoRequested);
    connect(settings_, &SettingsPanel::redoRequested, this, &MainWindow::onRedoRequested);
    connect(settings_, &SettingsPanel::nextTurnRequested, this, &MainWindow::onNextTurnRequested);
    connect(settings_, &SettingsPanel::resetViewRequested, this, &MainWindow::onResetViewRequested);

    settings_->setRulesToUi(cfg_.rules);
    settings_->setAiSettings(cfg_.aiEnabled, cfg_.aiPlayer, cfg_.aiCandidateRadius);

    aiEnabled_ = cfg_.aiEnabled;
    aiPlayer_ = cfg_.aiPlayer;
    aiRadius_ = cfg_.aiCandidateRadius;

    startNewGame(settings_->rulesFromUi());

    statusBar()->showMessage("Ready");
}

void MainWindow::onNewGameRequested() {
    startNewGame(settings_->rulesFromUi());
}

void MainWindow::startNewGame(const engine::RuleSet& rules) {
    engine::RuleSet r = rules;
    const auto warnings = r.validateAndFix();
    if (!warnings.empty()) {
        QString msg;
        for (const auto& w : warnings) msg += QString::fromStdString(w) + "\n";
        statusBar()->showMessage("Rules normalized. See warnings dialog.", 3000);
        QMessageBox::information(this, "Rule warnings", msg.trimmed());
    }

    game_.newGame(r, engine::GameState::createBoard(r));

    view_->setFiniteBounds(
        r.topology == engine::BoardTopology::Finite ? r.width : 0,
        r.topology == engine::BoardTopology::Finite ? r.height : 0
    );

    aiEnabled_ = settings_->aiEnabled();
    aiPlayer_ = settings_->aiPlayer();
    aiRadius_ = settings_->aiCandidateRadius();
    ai_ = engine::SimpleAI(engine::SimpleAI::Settings{aiRadius_, 600, 0});

    rebuildScene();
    updateUi();
    ensureAiMoveIfNeeded();
}

void MainWindow::rebuildScene() {
    scene_->clear();
    markItems_.clear();

    scene_->setSceneRect(boardSceneRect());

    grid_ = new GridItem(scene_->sceneRect(), cellSize_, game_.rules().topology == engine::BoardTopology::Finite);
    scene_->addItem(grid_);

    lastMoveHighlight_ = scene_->addRect(QRectF(), QPen(QColor(255, 170, 0), 2.0));
    lastMoveHighlight_->setZValue(5);
    lastMoveHighlight_->hide();

    syncSceneWithBoard();
    view_->resetViewToRect(scene_->sceneRect());
}

void MainWindow::syncSceneWithBoard() {
    const auto occ = game_.board().occupied();
    for (const auto& [coord, p] : occ) {
        const QRectF rect(coord.x * cellSize_, coord.y * cellSize_, cellSize_, cellSize_);
        auto* item = new MarkItem(p, rect);
        scene_->addItem(item);
        markItems_[coord] = item;
    }

    if (game_.lastMove()) {
        const auto lm = game_.lastMove().value();
        const QRectF rect(lm.x * cellSize_, lm.y * cellSize_, cellSize_, cellSize_);
        lastMoveHighlight_->setRect(rect.adjusted(1, 1, -1, -1));
        lastMoveHighlight_->show();
    } else {
        lastMoveHighlight_->hide();
    }
}

void MainWindow::updateUi() {
    settings_->updateFromGameState(game_);

    const bool aiVsAi = isAiVsAiModeActive();
    settings_->setNextTurnVisible(aiVsAi);
    settings_->setNextTurnEnabled(aiVsAi && !game_.isGameOver());
}
bool MainWindow::isAiVsAiModeActive() const {
    // Convention: aiEnabled=true + aiPlayer=None => both sides are controlled by AI.
    return aiEnabled_ && (aiPlayer_ == engine::Player::None);
}

void MainWindow::onCellClicked(int x, int y) {
    if (isAiVsAiModeActive()) {
        statusBar()->showMessage("AI vs AI mode: use 'Next Turn' to advance.", 2000);
        return;
    }

    if (aiEnabled_ && game_.currentPlayer() == aiPlayer_) {
        statusBar()->showMessage("AI turn: input ignored.", 1500);
        return;
    }

    const engine::Coord c{x, y};
    const auto res = game_.tryMakeMove(c);
    if (!res.ok) {
        statusBar()->showMessage(QString::fromStdString(res.message), 2000);
        return;
    }

    const engine::Player p = game_.board().get(c);
    const QRectF rect(x * cellSize_, y * cellSize_, cellSize_, cellSize_);
    auto* item = new MarkItem(p, rect);
    scene_->addItem(item);
    markItems_[c] = item;

    if (lastMoveHighlight_) {
        lastMoveHighlight_->setRect(rect.adjusted(1, 1, -1, -1));
        lastMoveHighlight_->show();
    }

    updateSceneRectForTopology();
    updateUi();

    if (game_.isGameOver()) {
        GameOverDialog dlg(game_, this);
        dlg.exec();
        return;
    }

    ensureAiMoveIfNeeded();
}

void MainWindow::onUndoRequested() {
    const auto last = game_.lastMove(); // last move before undo
    if (!game_.undo()) return;

    if (last) {
        auto it = markItems_.find(*last);
        if (it != markItems_.end()) {
            scene_->removeItem(it->second);
            delete it->second;
            markItems_.erase(it);
        }
    }

    if (game_.lastMove()) {
        const auto lm = game_.lastMove().value();
        const QRectF rect(lm.x * cellSize_, lm.y * cellSize_, cellSize_, cellSize_);
        lastMoveHighlight_->setRect(rect.adjusted(1, 1, -1, -1));
        lastMoveHighlight_->show();
    } else {
        lastMoveHighlight_->hide();
    }

    updateSceneRectForTopology();
    updateUi();
}

void MainWindow::onRedoRequested() {
    if (!game_.redo()) return;

    if (game_.lastMove()) {
        const auto lm = game_.lastMove().value();
        if (markItems_.find(lm) == markItems_.end()) {
            const engine::Player p = game_.board().get(lm);
            const QRectF rect(lm.x * cellSize_, lm.y * cellSize_, cellSize_, cellSize_);
            auto* item = new MarkItem(p, rect);
            scene_->addItem(item);
            markItems_[lm] = item;
        }
        lastMoveHighlight_->setRect(QRectF(lm.x * cellSize_, lm.y * cellSize_, cellSize_, cellSize_).adjusted(1, 1, -1, -1));
        lastMoveHighlight_->show();
    } else {
        lastMoveHighlight_->hide();
    }

    updateSceneRectForTopology();
    updateUi();

    if (game_.isGameOver()) {
        GameOverDialog dlg(game_, this);
        dlg.exec();
    } else {
        ensureAiMoveIfNeeded();
    }
}

void MainWindow::onResetViewRequested() {
    view_->resetViewToRect(scene_->sceneRect());
}
void MainWindow::onNextTurnRequested() {
    // "AI vs AI" в твоём проекте = AI включён, но aiPlayer() возвращает None
    // (см. SettingsPanel::aiPlayer()).
    if (!(aiEnabled_ && aiPlayer_ == engine::Player::None)) {
        statusBar()->showMessage("Next Turn is available only in AI vs AI mode.", 2000);
        return;
    }
    if (game_.isGameOver()) {
        statusBar()->showMessage("Game is already over.", 2000);
        return;
    }

    // Через event loop, чтобы UI не подвисал и было похоже на остальные AI-вызовы
    QTimer::singleShot(0, this, [this]() {
        if (!(aiEnabled_ && aiPlayer_ == engine::Player::None)) return;
        if (game_.isGameOver()) return;

        const engine::Player p = game_.currentPlayer();

        auto mv = ai_.chooseMove(game_, p);
        if (!mv) {
            statusBar()->showMessage("AI: no legal move found.", 2000);
            return;
        }

        const auto res = game_.tryMakeMove(*mv);
        if (!res.ok) {
            statusBar()->showMessage(
                QString("AI move failed: %1").arg(QString::fromStdString(res.message)),
                2000
            );
            return;
        }

        const engine::Coord c = *mv;
        const engine::Player placed = game_.board().get(c);
        const QRectF rect(c.x * cellSize_, c.y * cellSize_, cellSize_, cellSize_);

        auto* item = new MarkItem(placed, rect);
        scene_->addItem(item);
        markItems_[c] = item;

        if (lastMoveHighlight_) {
            lastMoveHighlight_->setRect(rect.adjusted(1, 1, -1, -1));
            lastMoveHighlight_->show();
        }

        updateSceneRectForTopology();
        updateUi();

        if (game_.isGameOver()) {
            GameOverDialog dlg(game_, this);
            dlg.exec();
        }
    });
}

void MainWindow::ensureAiMoveIfNeeded() {
    if (!aiEnabled_) return;
    if (game_.isGameOver()) return;

    // In AI vs AI mode we advance moves manually with the "Next Turn" button.
    if (isAiVsAiModeActive()) return;

    if (aiPlayer_ == engine::Player::None) return;
    if (game_.currentPlayer() != aiPlayer_) return;

    QTimer::singleShot(0, this, [this]() { performAiMove(aiPlayer_); });
}

void MainWindow::performAiMove(engine::Player aiPlayer) {
    if (!aiEnabled_) return;
    if (game_.isGameOver()) return;
    if (aiPlayer == engine::Player::None) return;
    if (game_.currentPlayer() != aiPlayer) return;

    auto mv = ai_.chooseMove(game_, aiPlayer);
    if (!aiEnabled_ || game_.isGameOver() || game_.currentPlayer() != aiPlayer_) return;

    if (!mv) {
        statusBar()->showMessage("AI: no legal move found.", 2000);
        return;
    }

    const auto res = game_.tryMakeMove(*mv);
    if (!res.ok) {
        statusBar()->showMessage(QString("AI move failed: %1").arg(QString::fromStdString(res.message)), 2000);
        return;
    }

    const engine::Coord c = *mv;
    const engine::Player p = game_.board().get(c);
    const QRectF rect(c.x * cellSize_, c.y * cellSize_, cellSize_, cellSize_);

    auto* item = new MarkItem(p, rect);
    scene_->addItem(item);
    markItems_[c] = item;

    lastMoveHighlight_->setRect(rect.adjusted(1, 1, -1, -1));
    lastMoveHighlight_->show();

    updateSceneRectForTopology();
    updateUi();

    if (game_.isGameOver()) {
        GameOverDialog dlg(game_, this);
        dlg.exec();
    }
}

void MainWindow::updateSceneRectForTopology() {
    const QRectF r = boardSceneRect();
    scene_->setSceneRect(r);
    if (grid_) grid_->setRect(r);
}

QRectF MainWindow::boardSceneRect() const {
    const auto& r = game_.rules();

    if (r.topology == engine::BoardTopology::Finite) {
        return QRectF(0, 0, r.width * cellSize_, r.height * cellSize_);
    }

    const auto occ = game_.board().occupied();

    int minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (occ.empty()) {
        minX = -20; maxX = 20;
        minY = -20; maxY = 20;
    } else {
        minX = occ[0].first.x; maxX = occ[0].first.x;
        minY = occ[0].first.y; maxY = occ[0].first.y;
        for (const auto& [c, p] : occ) {
            (void)p;
            minX = std::min(minX, c.x);
            maxX = std::max(maxX, c.x);
            minY = std::min(minY, c.y);
            maxY = std::max(maxY, c.y);
        }
        constexpr int margin = 20;
        minX -= margin; maxX += margin;
        minY -= margin; maxY += margin;
    }

    const qreal left = static_cast<qreal>(minX) * cellSize_;
    const qreal top = static_cast<qreal>(minY) * cellSize_;
    const qreal right = static_cast<qreal>(maxX + 1) * cellSize_;
    const qreal bottom = static_cast<qreal>(maxY + 1) * cellSize_;
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}