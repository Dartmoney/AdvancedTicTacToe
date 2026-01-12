//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_SCORING_H
#define TIKTAKTOE_SCORING_H
#pragma once

#include "Board.h"
#include "RuleSet.h"

namespace engine {

    struct MoveDelta {
        int linesDelta = 0;
        long long scoreDelta = 0;
        int maxRunLen = 1; // maximum contiguous run length after placing the move (for classic rule)
    };

    class Scoring {
    public:
        // Computes how "lines" and "score" will change if Player p places at empty cell c.
        // Incremental: only analyzes 4 directions around the move.
        static MoveDelta computeMoveDelta(const IBoard& board, Coord c, Player p, const RuleSet& rules);
    };

} // namespace engine

#endif //TIKTAKTOE_SCORING_H