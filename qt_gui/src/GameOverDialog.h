#ifndef TIKTAKTOE_GAMEOVERDIALOG_H
#define TIKTAKTOE_GAMEOVERDIALOG_H
#pragma once

#include <QDialog>

#include <engine/GameState.h>

class GameOverDialog : public QDialog {
    Q_OBJECT
public:
    explicit GameOverDialog(const engine::GameState& state, QWidget* parent = nullptr);

private:
    void buildUi(const engine::GameState& state);
};

#endif