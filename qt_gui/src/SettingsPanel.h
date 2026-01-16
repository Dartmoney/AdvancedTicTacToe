#ifndef TIKTAKTOE_SETTINGSPANEL_H
#define TIKTAKTOE_SETTINGSPANEL_H
#pragma once

#include <QWidget>

#include <engine/GameState.h>
#include <engine/RuleSet.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

class SettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPanel(QWidget* parent = nullptr);

    engine::RuleSet rulesFromUi() const;
    void setRulesToUi(const engine::RuleSet& rules);

    bool aiEnabled() const;
    engine::Player aiPlayer() const;
    int aiCandidateRadius() const;

    void setAiSettings(bool enabled, engine::Player aiPlayer, int radius);

    void updateFromGameState(const engine::GameState& state);

    signals:
        void newGameRequested();
    void undoRequested();
    void redoRequested();
    void resetViewRequested();

private slots:
    void onTopologyChanged(int);
    void onPresetChanged(int);
    void onRuleTogglesChanged();
    void onCostModeChanged(int);
    void onAiModeChanged(int);

private:
    void buildUi();
    void updateUiEnabledStates();

    QComboBox* topologyCombo_ = nullptr;
    QComboBox* presetCombo_ = nullptr;
    QSpinBox* widthSpin_ = nullptr;
    QSpinBox* heightSpin_ = nullptr;

    QSpinBox* nSpin_ = nullptr;
    QComboBox* winRuleCombo_ = nullptr;
    QComboBox* lineModeCombo_ = nullptr;
    QCheckBox* countSubsegmentsCheck_ = nullptr;

    QCheckBox* weightsCheck_ = nullptr;
    QSpinBox* targetScoreSpin_ = nullptr;

    QSpinBox* maxMovesSpin_ = nullptr;

    QCheckBox* costsCheck_ = nullptr;
    QComboBox* costModeCombo_ = nullptr;
    QSpinBox* budgetSpin_ = nullptr;

    QComboBox* aiCombo_ = nullptr;
    QSpinBox* aiRadiusSpin_ = nullptr;

    QPushButton* newGameBtn_ = nullptr;
    QPushButton* undoBtn_ = nullptr;
    QPushButton* redoBtn_ = nullptr;
    QPushButton* resetViewBtn_ = nullptr;

    QLabel* statusLabel_ = nullptr;
    QLabel* statsLabel_ = nullptr;
};

#endif