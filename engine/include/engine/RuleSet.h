//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_RULESET_H
#define TIKTAKTOE_RULESET_H
#pragma once

#include <string>
#include <vector>

#include "CellValueFunction.h"

namespace engine {

    enum class BoardTopology { Finite, Infinite };
    enum class LineLengthMode { ExactN, AtLeastN };
    enum class CostMode { CostFromScore, CostFromBudget };

    struct RuleSet {
        // Board
        BoardTopology topology = BoardTopology::Finite;
        int width = 10;
        int height = 10;

        // Line definition
        int N = 5;
        LineLengthMode lineMode = LineLengthMode::AtLeastN;
        bool countSubsegments = false;

        // Win/goal rules
        bool classicWin = false;     // first who builds a line of length >= N wins immediately
        bool maximizeLines = true;   // at end-of-game: primary objective is to maximize number of lines

        // Weights & scoring
        bool weightsEnabled = false;
        CellValueFunction weightFunction = CellValueFunction::constantFunc(1);
        long long targetScore = 0;   // if >0 and weightsEnabled: reaching it ends the game immediately

        // Move limit (0 = unlimited)
        int maxMoves = 0;

        // Move costs
        bool moveCostsEnabled = false;
        CellValueFunction costFunction = CellValueFunction::constantFunc(0);
        CostMode costMode = CostMode::CostFromBudget;
        long long initialBudget = 0; // meaningful when costMode == CostFromBudget; -1 means unlimited (handled in GameState)

        // Validation (also normalizes some values). Returns list of warnings.
        std::vector<std::string> validateAndFix();
    };

    std::string toString(BoardTopology t);
    std::string toString(LineLengthMode m);
    std::string toString(CostMode m);

} // namespace engine

#endif //TIKTAKTOE_RULESET_H