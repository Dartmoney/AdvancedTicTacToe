#include "AppConfig.h"

#include <QFileInfo>
#include <QSettings>
#include <QStringList>

#include <engine/CellValueFunction.h>

namespace {

engine::BoardTopology parseTopology(const QString& s, bool* ok = nullptr) {
    const QString t = s.trimmed().toLower();
    if (t == "finite") { if (ok) *ok = true; return engine::BoardTopology::Finite; }
    if (t == "infinite") { if (ok) *ok = true; return engine::BoardTopology::Infinite; }
    if (ok) *ok = false;
    return engine::BoardTopology::Finite;
}

engine::LineLengthMode parseLineMode(const QString& s, bool* ok = nullptr) {
    const QString t = s.trimmed().toLower();
    if (t == "exact") { if (ok) *ok = true; return engine::LineLengthMode::ExactN; }
    if (t == "atleast") { if (ok) *ok = true; return engine::LineLengthMode::AtLeastN; }
    if (ok) *ok = false;
    return engine::LineLengthMode::AtLeastN;
}

engine::CostMode parseCostMode(const QString& s, bool* ok = nullptr) {
    const QString t = s.trimmed().toLower();
    if (t == "score") { if (ok) *ok = true; return engine::CostMode::CostFromScore; }
    if (t == "budget") { if (ok) *ok = true; return engine::CostMode::CostFromBudget; }
    if (ok) *ok = false;
    return engine::CostMode::CostFromBudget;
}

engine::Player parsePlayerXO(const QString& s, bool* ok = nullptr) {
    const QString t = s.trimmed().toUpper();
    if (t == "X") { if (ok) *ok = true; return engine::Player::X; }
    if (t == "O") { if (ok) *ok = true; return engine::Player::O; }
    if (t == "BOTH" || t == "AI" || t == "AIVSAI" || t == "AI VS AI") {
        if (ok) *ok = true;
        return engine::Player::None; // convention: enabled + None => AI vs AI
    }
    if (ok) *ok = false;
    return engine::Player::O;
}

engine::CellValueFunction::Type parseCellFuncType(const QString& s, bool* ok = nullptr) {
    const QString t = s.trimmed().toLower();
    if (t == "constant" || t == "const") { if (ok) *ok = true; return engine::CellValueFunction::Type::Constant; }
    if (t == "manhattan") { if (ok) *ok = true; return engine::CellValueFunction::Type::Manhattan; }
    if (t == "chebyshev") { if (ok) *ok = true; return engine::CellValueFunction::Type::Chebyshev; }
    if (t == "radial2" || t == "radialsquared") { if (ok) *ok = true; return engine::CellValueFunction::Type::RadialSquared; }
    if (t == "table") { if (ok) *ok = true; return engine::CellValueFunction::Type::Table; }
    if (ok) *ok = false;
    return engine::CellValueFunction::Type::Constant;
}

std::vector<int> parseTable(const QString& s, int& w, int& h, QStringList& warnings) {
    w = 0;
    h = 0;
    std::vector<int> out;

    const QString trimmed = s.trimmed();
    if (trimmed.isEmpty()) return out;

    const QStringList rows = trimmed.split(';', Qt::SkipEmptyParts);
    h = rows.size();
    if (h <= 0) return out;

    int expectedW = -1;
    for (int y = 0; y < h; ++y) {
        const QStringList cols = rows[y].split(',', Qt::SkipEmptyParts);
        if (expectedW < 0) expectedW = cols.size();
        if (cols.size() != expectedW) {
            warnings << QString("Table row %1 has %2 columns, expected %3. Will pad with zeros.")
                            .arg(y).arg(cols.size()).arg(expectedW);
        }
    }

    w = std::max(0, expectedW);
    out.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), 0);

    for (int y = 0; y < h; ++y) {
        const QStringList cols = rows[y].split(',', Qt::SkipEmptyParts);
        for (int x = 0; x < w; ++x) {
            int v = 0;
            if (x < cols.size()) v = cols[x].trimmed().toInt();
            out[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] = v;
        }
    }

    return out;
}

void applyCellValueFunction(engine::CellValueFunction& f,
                            QSettings& s,
                            const QString& prefix,
                            QStringList& warnings) {
    if (s.contains(prefix + ".type")) {
        bool ok = false;
        const auto t = parseCellFuncType(s.value(prefix + ".type").toString(), &ok);
        if (!ok) warnings << QString("%1.type is invalid. Using default.").arg(prefix);
        else f.type = t;
    }

    if (s.contains(prefix + ".constant")) f.constant = s.value(prefix + ".constant").toInt();
    if (s.contains(prefix + ".scale")) f.scale = s.value(prefix + ".scale").toInt();
    if (s.contains(prefix + ".offset")) f.offset = s.value(prefix + ".offset").toInt();

    if (s.contains(prefix + ".origin_x")) f.originX = s.value(prefix + ".origin_x").toInt();
    if (s.contains(prefix + ".origin_y")) f.originY = s.value(prefix + ".origin_y").toInt();

    if (s.contains(prefix + ".default")) f.defaultValue = s.value(prefix + ".default").toInt();
    if (s.contains(prefix + ".tableOffsetX")) f.tableOffsetX = s.value(prefix + ".tableOffsetX").toInt();
    if (s.contains(prefix + ".tableOffsetY")) f.tableOffsetY = s.value(prefix + ".tableOffsetY").toInt();

    if (f.type == engine::CellValueFunction::Type::Table) {
        const QString tableStr = s.value(prefix + ".table", "").toString();
        int w = 0, h = 0;
        auto table = parseTable(tableStr, w, h, warnings);
        f.tableWidth = w;
        f.tableHeight = h;
        f.table = std::move(table);
    }
}

void applyBoolPair(const QCommandLineParser& parser, const char* onOpt, const char* offOpt, bool& value) {
    if (parser.isSet(onOpt)) value = true;
    if (parser.isSet(offOpt)) value = false;
}

}

AppConfig AppConfig::fromParser(const QCommandLineParser& parser) {
    AppConfig cfg;

    cfg.rules.topology = engine::BoardTopology::Finite;
    cfg.rules.width = 10;
    cfg.rules.height = 10;
    cfg.rules.N = 5;

    cfg.rules.classicWin = false;
    cfg.rules.maximizeLines = true;
    cfg.rules.lineMode = engine::LineLengthMode::AtLeastN;
    cfg.rules.countSubsegments = false;

    cfg.rules.weightsEnabled = false;
    cfg.rules.weightFunction = engine::CellValueFunction::constantFunc(1);
    cfg.rules.targetScore = 0;

    cfg.rules.maxMoves = 0;

    cfg.rules.moveCostsEnabled = false;
    cfg.rules.costFunction = engine::CellValueFunction::constantFunc(0);
    cfg.rules.costMode = engine::CostMode::CostFromBudget;
    cfg.rules.initialBudget = 0;

    cfg.aiEnabled = false;
    cfg.aiPlayer = engine::Player::O;
    cfg.aiCandidateRadius = 2;

    cfg.cellSizePx = 40;

    if (parser.isSet("config")) {
        cfg.configFile = parser.value("config");
        if (!QFileInfo::exists(cfg.configFile)) {
            cfg.warnings << QString("Config file not found: %1").arg(cfg.configFile);
        } else {
            QSettings s(cfg.configFile, QSettings::IniFormat);

            s.beginGroup("rules");

            if (s.contains("topology")) {
                bool ok = false;
                cfg.rules.topology = parseTopology(s.value("topology").toString(), &ok);
                if (!ok) cfg.warnings << "rules.topology invalid; using default.";
            }

            cfg.rules.width = s.value("width", cfg.rules.width).toInt();
            cfg.rules.height = s.value("height", cfg.rules.height).toInt();
            cfg.rules.N = s.value("N", cfg.rules.N).toInt();

            cfg.rules.classicWin = s.value("classicWin", cfg.rules.classicWin).toBool();
            cfg.rules.maximizeLines = s.value("maximizeLines", cfg.rules.maximizeLines).toBool();

            if (s.contains("lineMode")) {
                bool ok = false;
                cfg.rules.lineMode = parseLineMode(s.value("lineMode").toString(), &ok);
                if (!ok) cfg.warnings << "rules.lineMode invalid; using default.";
            }

            cfg.rules.countSubsegments = s.value("countSubsegments", cfg.rules.countSubsegments).toBool();

            cfg.rules.weightsEnabled = s.value("weightsEnabled", cfg.rules.weightsEnabled).toBool();
            cfg.rules.targetScore = s.value("targetScore", static_cast<qlonglong>(cfg.rules.targetScore)).toLongLong();

            cfg.rules.maxMoves = s.value("maxMoves", cfg.rules.maxMoves).toInt();

            cfg.rules.moveCostsEnabled = s.value("moveCostsEnabled", cfg.rules.moveCostsEnabled).toBool();

            if (s.contains("costMode")) {
                bool ok = false;
                cfg.rules.costMode = parseCostMode(s.value("costMode").toString(), &ok);
                if (!ok) cfg.warnings << "rules.costMode invalid; using default.";
            }

            cfg.rules.initialBudget = s.value("initialBudget", static_cast<qlonglong>(cfg.rules.initialBudget)).toLongLong();

            applyCellValueFunction(cfg.rules.weightFunction, s, "weight", cfg.warnings);
            applyCellValueFunction(cfg.rules.costFunction, s, "cost", cfg.warnings);

            s.endGroup();

            s.beginGroup("ai");
            cfg.aiEnabled = s.value("enabled", cfg.aiEnabled).toBool();
            if (s.contains("player")) {
                bool ok = false;
                cfg.aiPlayer = parsePlayerXO(s.value("player").toString(), &ok);
                if (!ok) cfg.warnings << "ai.player invalid; using O.";
            }
            cfg.aiCandidateRadius = s.value("candidateRadius", cfg.aiCandidateRadius).toInt();
            s.endGroup();

            s.beginGroup("ui");
            cfg.cellSizePx = s.value("cellSizePx", cfg.cellSizePx).toInt();
            s.endGroup();
        }
    }

    if (parser.isSet("topology")) {
        bool ok = false;
        cfg.rules.topology = parseTopology(parser.value("topology"), &ok);
        if (!ok) cfg.warnings << "--topology invalid; using previous value.";
    }

    if (parser.isSet("width")) cfg.rules.width = parser.value("width").toInt();
    if (parser.isSet("height")) cfg.rules.height = parser.value("height").toInt();
    if (parser.isSet("N")) cfg.rules.N = parser.value("N").toInt();

    applyBoolPair(parser, "classic", "no-classic", cfg.rules.classicWin);
    applyBoolPair(parser, "maximize-lines", "no-maximize-lines", cfg.rules.maximizeLines);
    applyBoolPair(parser, "weights", "no-weights", cfg.rules.weightsEnabled);
    applyBoolPair(parser, "move-costs", "no-move-costs", cfg.rules.moveCostsEnabled);
    applyBoolPair(parser, "count-subsegments", "no-count-subsegments", cfg.rules.countSubsegments);

    if (parser.isSet("line-mode")) {
        bool ok = false;
        cfg.rules.lineMode = parseLineMode(parser.value("line-mode"), &ok);
        if (!ok) cfg.warnings << "--line-mode invalid; using previous value.";
    }

    if (parser.isSet("target-score")) cfg.rules.targetScore = parser.value("target-score").toLongLong();
    if (parser.isSet("max-moves")) cfg.rules.maxMoves = parser.value("max-moves").toInt();

    if (parser.isSet("cost-mode")) {
        bool ok = false;
        cfg.rules.costMode = parseCostMode(parser.value("cost-mode"), &ok);
        if (!ok) cfg.warnings << "--cost-mode invalid; using previous value.";
    }

    if (parser.isSet("budget")) cfg.rules.initialBudget = parser.value("budget").toLongLong();

    if (parser.isSet("ai")) {
        const QString mode = parser.value("ai").trimmed().toLower();
        if (mode == "none") {
            cfg.aiEnabled = false;
        } else if (mode == "x") {
            cfg.aiEnabled = true;
            cfg.aiPlayer = engine::Player::X;
        } else if (mode == "o") {
            cfg.aiEnabled = true;
            cfg.aiPlayer = engine::Player::O;
        } else if (mode == "both" || mode == "aivsai" || mode == "ai-vs-ai") {
            cfg.aiEnabled = true;
            cfg.aiPlayer = engine::Player::None; // convention: enabled + None => AI vs AI
        } else {
            cfg.warnings << "--ai invalid. Use none|X|O|both.";
        }
    }

    if (parser.isSet("ai-radius")) cfg.aiCandidateRadius = parser.value("ai-radius").toInt();
    if (parser.isSet("cell-size")) cfg.cellSizePx = parser.value("cell-size").toInt();

    const auto w = cfg.rules.validateAndFix();
    for (const auto& s : w) {
        cfg.warnings << QString::fromStdString(s);
    }

    return cfg;
}
