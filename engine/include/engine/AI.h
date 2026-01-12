#ifndef TIKTAKTOE_AI_H
#define TIKTAKTOE_AI_H
#pragma once

#include <optional>
#include <random>

#include "GameState.h"

namespace engine {

    class SimpleAI {
    public:
        struct Settings {
            int candidateRadius = 2;
            std::size_t maxCandidates = 600;
            unsigned seed = 0;
        };

        explicit SimpleAI(Settings s = {});

        std::optional<Coord> chooseMove(const GameState& state, Player aiPlayer);

    private:
        Settings s_;
        std::mt19937 rng_;

        long long evalMove(const GameState& state, Player p, Coord c) const;
    };

}

#endif