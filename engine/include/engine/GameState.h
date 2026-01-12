#ifndef TIKTAKTOE_GAMESTATE_H
#define TIKTAKTOE_GAMESTATE_H
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Board.h"
#include "Move.h"
#include "RuleSet.h"

namespace engine {

struct PlayerStats {
    long long score = 0;
    int lines = 0;
    long long budget = 0;
};

enum class GameResult { InProgress, Draw, WinX, WinO };
enum class EndReason { None, ClassicLine, TargetScore, MoveLimit, BoardFull, NoLegalMoves };

struct MoveOutcome {
    bool ok = false;
    std::string message;
};

class GameState {
public:
    GameState();
    GameState(RuleSet rules, std::unique_ptr<IBoard> board);

    void newGame(RuleSet rules, std::unique_ptr<IBoard> board);

    static std::unique_ptr<IBoard> createBoard(const RuleSet& rules);

    const RuleSet& rules() const noexcept { return rules_; }
    const IBoard& board() const noexcept { return *board_; }
    IBoard& board() noexcept { return *board_; }

    Player currentPlayer() const noexcept { return current_; }
    int moveCount() const noexcept { return moveCount_; }

    const PlayerStats& stats(Player p) const;
    PlayerStats& stats(Player p);

    bool isGameOver() const noexcept { return result_ != GameResult::InProgress; }
    GameResult result() const noexcept { return result_; }
    EndReason endReason() const noexcept { return reason_; }
    std::optional<Player> winner() const noexcept;

    std::optional<Coord> lastMove() const noexcept { return lastMove_; }
    int lastMoveCost() const noexcept { return lastMoveCost_; }

    MoveOutcome tryMakeMove(Coord c);

    bool canUndo() const noexcept { return !undoStack_.empty(); }
    bool canRedo() const noexcept { return !redoStack_.empty(); }
    bool undo();
    bool redo();

    bool isMoveLegal(Coord c) const;
    int moveCost(Coord c) const;
    long long cellWeight(Coord c) const;

    std::vector<Coord> generateCandidateMoves(int radius, std::size_t maxCandidates) const;

private:
    struct Snapshot {
        Player current{};
        int moveCount{};
        PlayerStats statsX{};
        PlayerStats statsO{};
        GameResult result{};
        EndReason reason{};
        std::optional<Coord> lastMove{};
        int lastMoveCost{};
    };

    struct MoveRecord {
        Move move{};
        Snapshot before{};
        Snapshot after{};
    };

    RuleSet rules_{};
    std::unique_ptr<IBoard> board_{};

    Player current_ = Player::X;
    int moveCount_ = 0;
    PlayerStats stats_[2]{};

    GameResult result_ = GameResult::InProgress;
    EndReason reason_ = EndReason::None;

    std::optional<Coord> lastMove_{};
    int lastMoveCost_ = 0;

    std::vector<MoveRecord> undoStack_;
    std::vector<MoveRecord> redoStack_;

    Snapshot makeSnapshot() const;
    void restoreSnapshot(const Snapshot& s);

    bool boardFull() const;
    bool hasAnyLegalMove(Player p) const;

    void evaluateEndOfGame(Player lastMover, int maxRunLenAfterMove);
    void finishByComparison(EndReason why);
};

std::string toString(GameResult r);
std::string toString(EndReason r);

}
#endif