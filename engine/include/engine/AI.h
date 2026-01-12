//
// Created by imako on 08.01.2026.
//

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
            unsigned seed = 0; // 0 => random_device
        };

        explicit SimpleAI(Settings s = {});

        std::optional<Coord> chooseMove(const GameState& state, Player aiPlayer);

    private:
        Settings s_;
        std::mt19937 rng_;

        long long evalMove(const GameState& state, Player p, Coord c) const;
    };

} // namespace engine

#endif //TIKTAKTOE_AI_H