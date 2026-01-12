#include "../include/engine/GameState.h"

#include "../include/engine/FiniteBoard.h"
#include "../include/engine/InfiniteBoard.h"
#include "../include/engine/Scoring.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <unordered_set>

namespace engine {

GameState::GameState() {
    RuleSet rules;
    newGame(rules, createBoard(rules));
}

GameState::GameState(RuleSet rules, std::unique_ptr<IBoard> board) {
    newGame(std::move(rules), std::move(board));
}

std::unique_ptr<IBoard> GameState::createBoard(const RuleSet& rules) {
    if (rules.topology == BoardTopology::Finite) {
        return std::make_unique<FiniteBoard>(rules.width, rules.height);
    }
    return std::make_unique<InfiniteBoard>();
}

void GameState::newGame(RuleSet rules, std::unique_ptr<IBoard> board) {
    rules_ = std::move(rules);
    rules_.validateAndFix();

    board_ = std::move(board);
    if (!board_) {
        board_ = createBoard(rules_);
    }

    current_ = Player::X;
    moveCount_ = 0;

    stats_[0] = PlayerStats{};
    stats_[1] = PlayerStats{};

    // budgets
    if (rules_.moveCostsEnabled && rules_.costMode == CostMode::CostFromBudget) {
        stats_[0].budget = rules_.initialBudget;
        stats_[1].budget = rules_.initialBudget;
    } else {
        stats_[0].budget = rules_.initialBudget;
        stats_[1].budget = rules_.initialBudget;
    }

    result_ = GameResult::InProgress;
    reason_ = EndReason::None;

    lastMove_.reset();
    lastMoveCost_ = 0;

    undoStack_.clear();
    redoStack_.clear();
}

const PlayerStats& GameState::stats(Player p) const {
    const int idx = playerIndex(p);
    if (idx < 0) {
        static PlayerStats dummy{};
        return dummy;
    }
    return stats_[idx];
}

PlayerStats& GameState::stats(Player p) {
    const int idx = playerIndex(p);
    if (idx < 0) {
        throw std::invalid_argument("stats(): player must be X or O");
    }
    return stats_[idx];
}

std::optional<Player> GameState::winner() const noexcept {
    switch (result_) {
        case GameResult::WinX: return Player::X;
        case GameResult::WinO: return Player::O;
        default: return std::nullopt;
    }
}

GameState::Snapshot GameState::makeSnapshot() const {
    Snapshot s;
    s.current = current_;
    s.moveCount = moveCount_;
    s.statsX = stats_[0];
    s.statsO = stats_[1];
    s.result = result_;
    s.reason = reason_;
    s.lastMove = lastMove_;
    s.lastMoveCost = lastMoveCost_;
    return s;
}

void GameState::restoreSnapshot(const Snapshot& s) {
    current_ = s.current;
    moveCount_ = s.moveCount;
    stats_[0] = s.statsX;
    stats_[1] = s.statsO;
    result_ = s.result;
    reason_ = s.reason;
    lastMove_ = s.lastMove;
    lastMoveCost_ = s.lastMoveCost;
}

bool GameState::boardFull() const {
    if (!board_->isFinite()) return false;
    const long long total = static_cast<long long>(board_->width()) * static_cast<long long>(board_->height());
    const long long occ = static_cast<long long>(board_->occupied().size());
    return occ >= total;
}

int GameState::moveCost(Coord c) const {
    if (!rules_.moveCostsEnabled) return 0;
    return rules_.costFunction.value(c);
}

long long GameState::cellWeight(Coord c) const {
    return rules_.weightFunction.value(c);
}

bool GameState::isMoveLegal(Coord c) const {
    if (isGameOver()) return false;

    if (board_->isFinite() && !board_->inBounds(c)) return false;
    if (!board_->isEmpty(c)) return false;

    if (rules_.moveCostsEnabled && rules_.costMode == CostMode::CostFromBudget) {
        const int cost = moveCost(c);
        const long long b = stats(current_).budget;
        // negative budget = unlimited
        if (b >= 0 && b < cost) return false;
    }

    return true;
}

bool GameState::hasAnyLegalMove(Player p) const {
    if (isGameOver()) return false;

    if (!rules_.moveCostsEnabled || rules_.costMode != CostMode::CostFromBudget) {
        if (!board_->isFinite()) return true;
        return !boardFull();
    }

    const long long b = stats(p).budget;
    if (b < 0) {
        if (!board_->isFinite()) return true;
        return !boardFull();
    }

    if (!board_->isFinite()) {
        // infinite: can't scan all. assume there exists a move, but practical games should use move limit or other stop.
        return b >= 0;
    }

    for (int y = 0; y < board_->height(); ++y) {
        for (int x = 0; x < board_->width(); ++x) {
            Coord c{x, y};
            if (board_->isEmpty(c) && moveCost(c) <= b) {
                return true;
            }
        }
    }
    return false;
}

MoveOutcome GameState::tryMakeMove(Coord c) {
    MoveOutcome out;

    if (isGameOver()) {
        out.ok = false;
        out.message = "Game is over.";
        return out;
    }

    if (board_->isFinite() && !board_->inBounds(c)) {
        out.ok = false;
        out.message = "Move is out of bounds.";
        return out;
    }

    if (!board_->isEmpty(c)) {
        out.ok = false;
        out.message = "Cell is already occupied.";
        return out;
    }

    int cost = moveCost(c);
    if (cost < 0) cost = 0;

    if (rules_.moveCostsEnabled && rules_.costMode == CostMode::CostFromBudget) {
        long long& b = stats(current_).budget;
        if (b >= 0 && b < cost) {
            out.ok = false;
            out.message = "Not enough budget for this move.";
            return out;
        }
    }

    MoveRecord rec;
    rec.before = makeSnapshot();

    const MoveDelta delta = Scoring::computeMoveDelta(*board_, c, current_, rules_);

    if (!board_->set(c, current_)) {
        out.ok = false;
        out.message = "Failed to apply move (board rejected).";
        return out;
    }

    PlayerStats& s = stats(current_);
    s.lines += delta.linesDelta;
    s.score += delta.scoreDelta;

    if (rules_.moveCostsEnabled) {
        if (rules_.costMode == CostMode::CostFromBudget) {
            if (s.budget >= 0) s.budget -= cost;
        } else {
            s.score -= cost;
        }
    }

    lastMove_ = c;
    lastMoveCost_ = cost;
    ++moveCount_;

    evaluateEndOfGame(current_, delta.maxRunLen);

    if (!isGameOver()) {
        current_ = other(current_);
    }

    rec.move = Move{c, rec.before.current, cost};
    rec.after = makeSnapshot();

    undoStack_.push_back(rec);
    redoStack_.clear();

    out.ok = true;
    out.message = "OK";
    return out;
}

bool GameState::undo() {
    if (undoStack_.empty()) return false;

    MoveRecord rec = undoStack_.back();
    undoStack_.pop_back();

    board_->clear(rec.move.coord);
    restoreSnapshot(rec.before);

    redoStack_.push_back(rec);
    return true;
}

bool GameState::redo() {
    if (redoStack_.empty()) return false;

    MoveRecord rec = redoStack_.back();
    redoStack_.pop_back();

    board_->set(rec.move.coord, rec.move.player);
    restoreSnapshot(rec.after);

    undoStack_.push_back(rec);
    return true;
}

void GameState::evaluateEndOfGame(Player lastMover, int maxRunLenAfterMove) {
    if (rules_.classicWin && maxRunLenAfterMove >= rules_.N) {
        result_ = (lastMover == Player::X) ? GameResult::WinX : GameResult::WinO;
        reason_ = EndReason::ClassicLine;
        return;
    }

    if (rules_.weightsEnabled && rules_.targetScore > 0) {
        if (stats(lastMover).score >= rules_.targetScore) {
            result_ = (lastMover == Player::X) ? GameResult::WinX : GameResult::WinO;
            reason_ = EndReason::TargetScore;
            return;
        }
    }

    if (rules_.topology == BoardTopology::Finite && boardFull()) {
        finishByComparison(EndReason::BoardFull);
        return;
    }

    if (rules_.maxMoves > 0 && moveCount_ >= rules_.maxMoves) {
        finishByComparison(EndReason::MoveLimit);
        return;
    }

    if (rules_.moveCostsEnabled && rules_.costMode == CostMode::CostFromBudget && rules_.topology == BoardTopology::Finite) {
        if (!hasAnyLegalMove(Player::X) && !hasAnyLegalMove(Player::O)) {
            finishByComparison(EndReason::NoLegalMoves);
            return;
        }
    }

    result_ = GameResult::InProgress;
    reason_ = EndReason::None;
}

void GameState::finishByComparison(EndReason why) {
    reason_ = why;

    const PlayerStats& x = stats_[0];
    const PlayerStats& o = stats_[1];

    auto decideByScore = [&]() -> GameResult {
        if (x.score > o.score) return GameResult::WinX;
        if (o.score > x.score) return GameResult::WinO;
        return GameResult::Draw;
    };

    if (rules_.maximizeLines) {
        if (x.lines > o.lines) { result_ = GameResult::WinX; return; }
        if (o.lines > x.lines) { result_ = GameResult::WinO; return; }

        // tie-break
        if (rules_.weightsEnabled || rules_.moveCostsEnabled) {
            result_ = decideByScore();
            return;
        }

        result_ = GameResult::Draw;
        return;
    }

    if (rules_.weightsEnabled || rules_.moveCostsEnabled) {
        result_ = decideByScore();
        return;
    }

    result_ = GameResult::Draw;
}

std::vector<Coord> GameState::generateCandidateMoves(int radius, std::size_t maxCandidates) const {
    std::vector<Coord> out;
    if (radius < 0) radius = 0;

    const auto occ = board_->occupied();
    if (occ.empty()) {
        if (board_->isFinite()) {
            out.push_back(Coord{board_->width() / 2, board_->height() / 2});
        } else {
            out.push_back(Coord{0, 0});
        }
        return out;
    }

    std::unordered_set<Coord, CoordHash> set;
    set.reserve(std::min<std::size_t>(maxCandidates * 2, 4096));

    for (const auto& [cell, p] : occ) {
        (void)p;
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                Coord c{cell.x + dx, cell.y + dy};
                if (board_->isFinite() && !board_->inBounds(c)) continue;
                if (!board_->isEmpty(c)) continue;
                set.insert(c);
            }
        }
    }

    out.assign(set.begin(), set.end());

    Coord ref = lastMove_.value_or(Coord{0, 0});
    std::sort(out.begin(), out.end(), [&](const Coord& a, const Coord& b) {
        const long long da = std::llabs(static_cast<long long>(a.x) - ref.x) + std::llabs(static_cast<long long>(a.y) - ref.y);
        const long long db = std::llabs(static_cast<long long>(b.x) - ref.x) + std::llabs(static_cast<long long>(b.y) - ref.y);
        if (da != db) return da < db;
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    });

    if (maxCandidates > 0 && out.size() > maxCandidates) {
        out.resize(maxCandidates);
    }
    return out;
}

std::string toString(GameResult r) {
    switch (r) {
        case GameResult::InProgress: return "in_progress";
        case GameResult::Draw: return "draw";
        case GameResult::WinX: return "win_X";
        case GameResult::WinO: return "win_O";
        default: return "unknown";
    }
}

std::string toString(EndReason r) {
    switch (r) {
        case EndReason::None: return "none";
        case EndReason::ClassicLine: return "classic_line";
        case EndReason::TargetScore: return "target_score";
        case EndReason::MoveLimit: return "move_limit";
        case EndReason::BoardFull: return "board_full";
        case EndReason::NoLegalMoves: return "no_legal_moves";
        default: return "unknown";
    }
}

} // namespace engine
