#ifndef TIKTAKTOE_SCORING_H
#define TIKTAKTOE_SCORING_H
#pragma once

#include "Board.h"
#include "RuleSet.h"

namespace engine {

    struct MoveDelta {
        int linesDelta = 0;
        long long scoreDelta = 0;
        int maxRunLen = 1;
    };

    class Scoring {
    public:
        static MoveDelta computeMoveDelta(const IBoard& board, Coord c, Player p, const RuleSet& rules);
    };

}

#endif