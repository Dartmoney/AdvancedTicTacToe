#include "GameOverDialog.h"
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

static QString resultToText(engine::GameResult r) {
    switch (r) {
        case engine::GameResult::WinX: return "X wins";
        case engine::GameResult::WinO: return "O wins";
        case engine::GameResult::Draw: return "Draw";
        default: return "In progress";
    }
}

static QString reasonToText(engine::EndReason r) {
    switch (r) {
        case engine::EndReason::ClassicLine: return "Classic line (>=N)";
        case engine::EndReason::TargetScore: return "Target score reached";
        case engine::EndReason::MoveLimit: return "Move limit reached";
        case engine::EndReason::BoardFull: return "Board full";
        case engine::EndReason::NoLegalMoves: return "No legal moves";
        default: return "None";
    }
}

GameOverDialog::GameOverDialog(const engine::GameState& state, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Game Over");
    buildUi(state);
}

void GameOverDialog::buildUi(const engine::GameState& state) {
    auto* root = new QVBoxLayout(this);

    auto* title = new QLabel(QString("<b>%1</b>").arg(resultToText(state.result())), this);
    root->addWidget(title);

    auto* reason = new QLabel(QString("Reason: %1").arg(reasonToText(state.endReason())), this);
    root->addWidget(reason);

    auto* table = new QTableWidget(2, 4, this);
    table->setHorizontalHeaderLabels({"Player", "Lines (per rules)", "Score", "Budget"});
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    const auto& sx = state.stats(engine::Player::X);
    const auto& so = state.stats(engine::Player::O);

    table->setItem(0, 0, new QTableWidgetItem("X"));
    table->setItem(0, 1, new QTableWidgetItem(QString::number(sx.lines)));
    table->setItem(0, 2, new QTableWidgetItem(QString::number(sx.score)));
    table->setItem(0, 3, new QTableWidgetItem(sx.budget < 0 ? "∞" : QString::number(sx.budget)));

    table->setItem(1, 0, new QTableWidgetItem("O"));
    table->setItem(1, 1, new QTableWidgetItem(QString::number(so.lines)));
    table->setItem(1, 2, new QTableWidgetItem(QString::number(so.score)));
    table->setItem(1, 3, new QTableWidgetItem(so.budget < 0 ? "∞" : QString::number(so.budget)));

    table->resizeColumnsToContents();
    root->addWidget(table);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    root->addWidget(buttons);
}
