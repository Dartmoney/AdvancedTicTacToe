#ifndef TIKTAKTOE_AI_H
#define TIKTAKTOE_AI_H


#include <optional>
#include <random>

#include "GameState.h"

namespace engine {

    class SimpleAI {
    public:
        struct Settings {
            int candidateRadius = 2;

            std::size_t maxCandidates = 600;

            std::size_t maxTopMoves = 40;

            std::size_t maxOpponentReplies = 40;

            bool enableTwoPly = true;

            bool enablePerfectClassic3x3 = true;

            unsigned seed = 0;
        };

        explicit SimpleAI(Settings s = {});

        std::optional<Coord> chooseMove(const GameState& state, Player aiPlayer);

    private:
        Settings s_;
        std::mt19937 rng_;
    };

} // namespace engine

#endif