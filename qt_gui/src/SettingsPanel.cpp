#include "SettingsPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QWidget(parent) {
    buildUi();
    updateUiEnabledStates();
}

void SettingsPanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    // Board group
    auto* boardGroup = new QGroupBox("Board", this);
    auto* boardForm = new QFormLayout(boardGroup);

    topologyCombo_ = new QComboBox(boardGroup);
    topologyCombo_->addItem("Finite");
    topologyCombo_->addItem("Infinite");
    boardForm->addRow("Topology:", topologyCombo_);

    presetCombo_ = new QComboBox(boardGroup);
    presetCombo_->addItem("10 x 10");
    presetCombo_->addItem("20 x 20");
    presetCombo_->addItem("30 x 30");
    presetCombo_->addItem("Custom");
    boardForm->addRow("Preset:", presetCombo_);

    widthSpin_ = new QSpinBox(boardGroup);
    widthSpin_->setRange(1, 500);
    widthSpin_->setValue(10);

    heightSpin_ = new QSpinBox(boardGroup);
    heightSpin_->setRange(1, 500);
    heightSpin_->setValue(10);

    boardForm->addRow("Width:", widthSpin_);
    boardForm->addRow("Height:", heightSpin_);

    root->addWidget(boardGroup);

    // Rules group
    auto* rulesGroup = new QGroupBox("Rules", this);
    auto* rulesForm = new QFormLayout(rulesGroup);

    nSpin_ = new QSpinBox(rulesGroup);
    nSpin_->setRange(1, 50);
    nSpin_->setValue(5);
    rulesForm->addRow("N (line length):", nSpin_);

    classicCheck_ = new QCheckBox("Classic first-to-N win", rulesGroup);
    classicCheck_->setChecked(false);
    rulesForm->addRow("", classicCheck_);

    maximizeLinesCheck_ = new QCheckBox("Maximize number of N-lines (new rule)", rulesGroup);
    maximizeLinesCheck_->setChecked(true);
    rulesForm->addRow("", maximizeLinesCheck_);

    lineModeCombo_ = new QComboBox(rulesGroup);
    lineModeCombo_->addItem("AtLeastN");
    lineModeCombo_->addItem("ExactN");
    rulesForm->addRow("Line mode:", lineModeCombo_);

    countSubsegmentsCheck_ = new QCheckBox("Count subsegments (AtLeastN)", rulesGroup);
    countSubsegmentsCheck_->setChecked(false);
    rulesForm->addRow("", countSubsegmentsCheck_);

    weightsCheck_ = new QCheckBox("Enable weights/scoring", rulesGroup);
    weightsCheck_->setChecked(false);
    rulesForm->addRow("", weightsCheck_);

    targetScoreSpin_ = new QSpinBox(rulesGroup);
    targetScoreSpin_->setRange(0, 1'000'000'000);
    targetScoreSpin_->setValue(0);
    rulesForm->addRow("Target score:", targetScoreSpin_);

    maxMovesSpin_ = new QSpinBox(rulesGroup);
    maxMovesSpin_->setRange(0, 1'000'000);
    maxMovesSpin_->setValue(0);
    rulesForm->addRow("Move limit (0=∞):", maxMovesSpin_);

    costsCheck_ = new QCheckBox("Enable move costs", rulesGroup);
    costsCheck_->setChecked(false);
    rulesForm->addRow("", costsCheck_);

    costModeCombo_ = new QComboBox(rulesGroup);
    costModeCombo_->addItem("From Budget");
    costModeCombo_->addItem("From Score");
    rulesForm->addRow("Cost mode:", costModeCombo_);

    budgetSpin_ = new QSpinBox(rulesGroup);
    budgetSpin_->setRange(-1, 1'000'000'000);
    budgetSpin_->setValue(0);
    budgetSpin_->setToolTip("-1 means unlimited budget in this demo.");
    rulesForm->addRow("Budget (-1=∞):", budgetSpin_);

    root->addWidget(rulesGroup);

    // AI group
    auto* aiGroup = new QGroupBox("AI (optional)", this);
    auto* aiForm = new QFormLayout(aiGroup);

    aiCombo_ = new QComboBox(aiGroup);
    aiCombo_->addItem("Human vs Human");
    aiCombo_->addItem("AI plays X");
    aiCombo_->addItem("AI plays O");
    aiForm->addRow("Mode:", aiCombo_);

    aiRadiusSpin_ = new QSpinBox(aiGroup);
    aiRadiusSpin_->setRange(1, 6);
    aiRadiusSpin_->setValue(2);
    aiForm->addRow("Candidate radius:", aiRadiusSpin_);

    root->addWidget(aiGroup);

    // Buttons
    auto* btnRow = new QHBoxLayout();
    newGameBtn_ = new QPushButton("New Game", this);
    undoBtn_ = new QPushButton("Undo", this);
    redoBtn_ = new QPushButton("Redo", this);
    resetViewBtn_ = new QPushButton("Reset View", this);

    btnRow->addWidget(newGameBtn_);
    btnRow->addWidget(undoBtn_);
    btnRow->addWidget(redoBtn_);
    btnRow->addWidget(resetViewBtn_);

    root->addLayout(btnRow);

    // Status/stats
    statusLabel_ = new QLabel(this);
    statusLabel_->setText("Ready");
    statusLabel_->setWordWrap(true);

    statsLabel_ = new QLabel(this);
    statsLabel_->setText("");
    statsLabel_->setWordWrap(true);

    root->addWidget(statusLabel_);
    root->addWidget(statsLabel_);
    root->addStretch(1);

    // Connections
    connect(topologyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPanel::onTopologyChanged);
    connect(presetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPanel::onPresetChanged);

    connect(classicCheck_, &QCheckBox::toggled, this, &SettingsPanel::onRuleTogglesChanged);
    connect(maximizeLinesCheck_, &QCheckBox::toggled, this, &SettingsPanel::onRuleTogglesChanged);
    connect(weightsCheck_, &QCheckBox::toggled, this, &SettingsPanel::onRuleTogglesChanged);
    connect(costsCheck_, &QCheckBox::toggled, this, &SettingsPanel::onRuleTogglesChanged);
    connect(countSubsegmentsCheck_, &QCheckBox::toggled, this, &SettingsPanel::onRuleTogglesChanged);
    connect(lineModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPanel::onRuleTogglesChanged);
    connect(costModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPanel::onCostModeChanged);
    connect(aiCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPanel::onAiModeChanged);

    connect(newGameBtn_, &QPushButton::clicked, this, &SettingsPanel::newGameRequested);
    connect(undoBtn_, &QPushButton::clicked, this, &SettingsPanel::undoRequested);
    connect(redoBtn_, &QPushButton::clicked, this, &SettingsPanel::redoRequested);
    connect(resetViewBtn_, &QPushButton::clicked, this, &SettingsPanel::resetViewRequested);
}

void SettingsPanel::onTopologyChanged(int) {
    updateUiEnabledStates();
}

void SettingsPanel::onPresetChanged(int idx) {
    // 0:10, 1:20, 2:30, 3:custom
    if (idx == 0) { widthSpin_->setValue(10); heightSpin_->setValue(10); }
    if (idx == 1) { widthSpin_->setValue(20); heightSpin_->setValue(20); }
    if (idx == 2) { widthSpin_->setValue(30); heightSpin_->setValue(30); }
    updateUiEnabledStates();
}

void SettingsPanel::onRuleTogglesChanged() {
    updateUiEnabledStates();
}

void SettingsPanel::onCostModeChanged(int) {
    updateUiEnabledStates();
}

void SettingsPanel::onAiModeChanged(int) {
    updateUiEnabledStates();
}

void SettingsPanel::updateUiEnabledStates() {
    const bool infinite = (topologyCombo_->currentIndex() == 1);

    presetCombo_->setEnabled(!infinite);
    const bool custom = (presetCombo_->currentIndex() == 3);

    widthSpin_->setEnabled(!infinite && custom);
    heightSpin_->setEnabled(!infinite && custom);

    const bool weights = weightsCheck_->isChecked();
    targetScoreSpin_->setEnabled(weights);

    const bool costs = costsCheck_->isChecked();
    costModeCombo_->setEnabled(costs);

    const bool budgetMode = (costModeCombo_->currentIndex() == 0);
    budgetSpin_->setEnabled(costs && budgetMode);

    const bool aiOn = (aiCombo_->currentIndex() != 0);
    aiRadiusSpin_->setEnabled(aiOn);

    // CountSubsegments only meaningful for AtLeastN
    const bool atleast = (lineModeCombo_->currentIndex() == 0);
    countSubsegmentsCheck_->setEnabled(atleast);
    if (!atleast) countSubsegmentsCheck_->setChecked(false);
}

engine::RuleSet SettingsPanel::rulesFromUi() const {
    engine::RuleSet r;

    const bool infinite = (topologyCombo_->currentIndex() == 1);
    r.topology = infinite ? engine::BoardTopology::Infinite : engine::BoardTopology::Finite;

    r.width = widthSpin_->value();
    r.height = heightSpin_->value();

    r.N = nSpin_->value();
    r.classicWin = classicCheck_->isChecked();
    r.maximizeLines = maximizeLinesCheck_->isChecked();

    const bool atleast = (lineModeCombo_->currentIndex() == 0);
    r.lineMode = atleast ? engine::LineLengthMode::AtLeastN : engine::LineLengthMode::ExactN;

    r.countSubsegments = countSubsegmentsCheck_->isChecked();

    r.weightsEnabled = weightsCheck_->isChecked();
    r.targetScore = targetScoreSpin_->value();
    // weightFunction stays default constant=1 (formula/table through config/CLI)

    r.maxMoves = maxMovesSpin_->value();

    r.moveCostsEnabled = costsCheck_->isChecked();
    const bool budgetMode = (costModeCombo_->currentIndex() == 0);
    r.costMode = budgetMode ? engine::CostMode::CostFromBudget : engine::CostMode::CostFromScore;
    r.initialBudget = budgetSpin_->value();
    // costFunction stays default constant=0 (formula/table through config/CLI)

    return r;
}

void SettingsPanel::setRulesToUi(const engine::RuleSet& r) {
    {
        QSignalBlocker b(topologyCombo_);
        topologyCombo_->setCurrentIndex(r.topology == engine::BoardTopology::Infinite ? 1 : 0);
    }

    // preset: try to match
    {
        QSignalBlocker b(presetCombo_);
        int preset = 3;
        if (r.width == 10 && r.height == 10) preset = 0;
        if (r.width == 20 && r.height == 20) preset = 1;
        if (r.width == 30 && r.height == 30) preset = 2;
        presetCombo_->setCurrentIndex(preset);
    }

    {
        QSignalBlocker b(widthSpin_);
        widthSpin_->setValue(std::max(1, r.width));
    }
    {
        QSignalBlocker b(heightSpin_);
        heightSpin_->setValue(std::max(1, r.height));
    }

    {
        QSignalBlocker b(nSpin_);
        nSpin_->setValue(std::max(1, r.N));
    }

    {
        QSignalBlocker b(classicCheck_);
        classicCheck_->setChecked(r.classicWin);
    }
    {
        QSignalBlocker b(maximizeLinesCheck_);
        maximizeLinesCheck_->setChecked(r.maximizeLines);
    }

    {
        QSignalBlocker b(lineModeCombo_);
        lineModeCombo_->setCurrentIndex(r.lineMode == engine::LineLengthMode::AtLeastN ? 0 : 1);
    }

    {
        QSignalBlocker b(countSubsegmentsCheck_);
        countSubsegmentsCheck_->setChecked(r.countSubsegments);
    }

    {
        QSignalBlocker b(weightsCheck_);
        weightsCheck_->setChecked(r.weightsEnabled);
    }
    {
        QSignalBlocker b(targetScoreSpin_);
        targetScoreSpin_->setValue(static_cast<int>(std::max<long long>(0, r.targetScore)));
    }

    {
        QSignalBlocker b(maxMovesSpin_);
        maxMovesSpin_->setValue(std::max(0, r.maxMoves));
    }

    {
        QSignalBlocker b(costsCheck_);
        costsCheck_->setChecked(r.moveCostsEnabled);
    }
    {
        QSignalBlocker b(costModeCombo_);
        costModeCombo_->setCurrentIndex(r.costMode == engine::CostMode::CostFromBudget ? 0 : 1);
    }
    {
        QSignalBlocker b(budgetSpin_);
        budgetSpin_->setValue(static_cast<int>(r.initialBudget));
    }

    updateUiEnabledStates();
}

bool SettingsPanel::aiEnabled() const {
    return aiCombo_->currentIndex() != 0;
}

engine::Player SettingsPanel::aiPlayer() const {
    if (aiCombo_->currentIndex() == 1) return engine::Player::X;
    if (aiCombo_->currentIndex() == 2) return engine::Player::O;
    return engine::Player::None;
}

int SettingsPanel::aiCandidateRadius() const {
    return aiRadiusSpin_->value();
}

void SettingsPanel::setAiSettings(bool enabled, engine::Player aiPlayer, int radius) {
    QSignalBlocker b1(aiCombo_);
    if (!enabled) {
        aiCombo_->setCurrentIndex(0);
    } else {
        if (aiPlayer == engine::Player::X) aiCombo_->setCurrentIndex(1);
        else aiCombo_->setCurrentIndex(2);
    }

    QSignalBlocker b2(aiRadiusSpin_);
    aiRadiusSpin_->setValue(std::max(1, radius));

    updateUiEnabledStates();
}

void SettingsPanel::updateFromGameState(const engine::GameState& state) {
    undoBtn_->setEnabled(state.canUndo());
    redoBtn_->setEnabled(state.canRedo());

    QString status;
    if (!state.isGameOver()) {
        status = QString("Turn: %1").arg(state.currentPlayer() == engine::Player::X ? "X" : "O");
    } else {
        QString res;
        if (state.result() == engine::GameResult::WinX) res = "X wins";
        else if (state.result() == engine::GameResult::WinO) res = "O wins";
        else res = "Draw";
        status = "Game Over: " + res;
    }
    statusLabel_->setText(status);

    const auto& sx = state.stats(engine::Player::X);
    const auto& so = state.stats(engine::Player::O);

    QString budgetX = (sx.budget < 0) ? "∞" : QString::number(sx.budget);
    QString budgetO = (so.budget < 0) ? "∞" : QString::number(so.budget);

    QString last;
    if (state.lastMove()) {
        const auto lm = state.lastMove().value();
        last = QString("Last move: (%1,%2), cost=%3").arg(lm.x).arg(lm.y).arg(state.lastMoveCost());
    } else {
        last = "Last move: none";
    }

    statsLabel_->setText(
        QString("X: lines=%1, score=%2, budget=%3\nO: lines=%4, score=%5, budget=%6\n%7")
            .arg(sx.lines).arg(sx.score).arg(budgetX)
            .arg(so.lines).arg(so.score).arg(budgetO)
            .arg(last)
    );
}
