#include "../include/engine/Scoring.h"

#include <algorithm>
#include <numeric>
#include <vector>

namespace engine {
namespace {

struct Contribution {
    int lines = 0;
    long long score = 0;
};

Contribution evaluateRun(const std::vector<int>& w, const RuleSet& rules) {
    Contribution c;
    const int R = static_cast<int>(w.size());
    const int N = std::max(1, rules.N);

    if (R <= 0) return c;

    auto sumWeights = [&]() -> long long {
        return std::accumulate(w.begin(), w.end(), 0LL);
    };

    switch (rules.lineMode) {
        case LineLengthMode::ExactN: {
            // ExactN: counts only maximal run exactly N (run length must be exactly N)
            if (R == N) {
                c.lines = 1;
                if (rules.weightsEnabled) {
                    c.score = sumWeights() * R;
                }
            }
            break;
        }

        case LineLengthMode::AtLeastN: {
            if (R >= N) {
                if (rules.countSubsegments) {
                    // CountSubsegments: number of N-windows inside the run
                    c.lines = R - N + 1;
                    if (rules.weightsEnabled) {
                        std::vector<long long> prefix(static_cast<std::size_t>(R) + 1, 0);
                        for (int i = 0; i < R; ++i) {
                            prefix[static_cast<std::size_t>(i) + 1] = prefix[static_cast<std::size_t>(i)] + w[static_cast<std::size_t>(i)];
                        }
                        long long s = 0;
                        for (int i = 0; i <= R - N; ++i) {
                            long long segSum = prefix[static_cast<std::size_t>(i) + N] - prefix[static_cast<std::size_t>(i)];
                            s += segSum * N;
                        }
                        c.score = s;
                    }
                } else {
                    // Single run counts as one line
                    c.lines = 1;
                    if (rules.weightsEnabled) {
                        c.score = sumWeights() * R;
                    }
                }
            }
            break;
        }
    }

    return c;
}

void gatherSide(const IBoard& board,
                Coord start,
                int dx,
                int dy,
                Player p,
                const CellValueFunction& func,
                std::vector<int>& outWeights) {
    outWeights.clear();
    Coord c = start;
    while (board.inBounds(c) && board.get(c) == p) {
        outWeights.push_back(func.value(c));
        c.x += dx;
        c.y += dy;
    }
}

} // namespace

MoveDelta Scoring::computeMoveDelta(const IBoard& board, Coord c, Player p, const RuleSet& rules) {
    MoveDelta total;

    if (board.isFinite() && !board.inBounds(c)) {
        return total;
    }
    if (board.get(c) != Player::None) {
        return total;
    }

    const auto& wFunc = rules.weightFunction;
    const int wCenter = wFunc.value(c);

    struct Dir { int dx; int dy; };
    const Dir dirs[4] = { {1,0}, {0,1}, {1,1}, {1,-1} };

    std::vector<int> leftW;
    std::vector<int> rightW;
    std::vector<int> mergedW;

    for (const auto& d : dirs) {
        gatherSide(board, Coord{c.x - d.dx, c.y - d.dy}, -d.dx, -d.dy, p, wFunc, leftW);
        gatherSide(board, Coord{c.x + d.dx, c.y + d.dy}, d.dx, d.dy, p, wFunc, rightW);

        mergedW.clear();
        mergedW.reserve(leftW.size() + 1 + rightW.size());

        // leftW collected near->far, need far->near to form correct order
        for (auto it = leftW.rbegin(); it != leftW.rend(); ++it) {
            mergedW.push_back(*it);
        }
        mergedW.push_back(wCenter);
        for (int w : rightW) {
            mergedW.push_back(w);
        }

        Contribution oldLeft = evaluateRun(leftW, rules);
        Contribution oldRight = evaluateRun(rightW, rules);
        Contribution newMerged = evaluateRun(mergedW, rules);

        total.linesDelta += newMerged.lines - oldLeft.lines - oldRight.lines;
        total.scoreDelta += newMerged.score - oldLeft.score - oldRight.score;
        total.maxRunLen = std::max(total.maxRunLen, static_cast<int>(mergedW.size()));
    }

    return total;
}

} // namespace engine
