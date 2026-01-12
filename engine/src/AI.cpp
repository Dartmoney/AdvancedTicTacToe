#include "../include/engine/AI.h"

#include "../include/engine/Scoring.h"

#include <limits>
#include <random>

namespace engine {

SimpleAI::SimpleAI(Settings s) : s_(s) {
    if (s_.seed == 0) {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    } else {
        rng_ = std::mt19937(s_.seed);
    }
}

long long SimpleAI::evalMove(const GameState& state, Player p, Coord c) const {
    const RuleSet& rules = state.rules();
    const IBoard& board = state.board();

    if (board.isFinite() && !board.inBounds(c)) {
        return std::numeric_limits<long long>::min() / 4;
    }
    if (!board.isEmpty(c)) {
        return std::numeric_limits<long long>::min() / 4;
    }
    if (!state.isMoveLegal(c)) {
        return std::numeric_limits<long long>::min() / 4;
    }

    const int cost = state.moveCost(c);

    const MoveDelta my = Scoring::computeMoveDelta(board, c, p, rules);
    const MoveDelta opp = Scoring::computeMoveDelta(board, c, other(p), rules);

    long long score = 0;

    if (rules.classicWin) {
        if (my.maxRunLen >= rules.N) score += 1'000'000'000'000LL;  // win now
        if (opp.maxRunLen >= rules.N) score += 500'000'000'000LL;   // block
    }

    score += static_cast<long long>(my.linesDelta) * 1'000'000LL;
    score += my.scoreDelta;

    // Prefer blocking opponent's line creation a bit
    score += static_cast<long long>(-opp.linesDelta) * 200'000LL;

    // Cost penalty
    score -= static_cast<long long>(cost) * 10'000LL;

    return score;
}

std::optional<Coord> SimpleAI::chooseMove(const GameState& state, Player aiPlayer) {
    if (state.isGameOver()) return std::nullopt;

    auto candidates = state.generateCandidateMoves(s_.candidateRadius, s_.maxCandidates);
    if (candidates.empty()) {
        if (state.board().isFinite()) {
            Coord c{state.board().width() / 2, state.board().height() / 2};
            if (state.isMoveLegal(c)) return c;
        }
        Coord c{0, 0};
        if (state.isMoveLegal(c)) return c;
        return std::nullopt;
    }

    std::uniform_int_distribution<int> noise(0, 9999);

    long long bestScore = std::numeric_limits<long long>::min();
    std::optional<Coord> bestMove;

    for (const auto& c : candidates) {
        long long s = evalMove(state, aiPlayer, c);
        s += noise(rng_);
        if (!bestMove || s > bestScore) {
            bestScore = s;
            bestMove = c;
        }
    }

    return bestMove;
}

}
