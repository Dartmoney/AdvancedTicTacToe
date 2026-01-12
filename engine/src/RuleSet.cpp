#include "../include/engine/RuleSet.h"

namespace engine {

std::vector<std::string> RuleSet::validateAndFix() {
    std::vector<std::string> warnings;

    if (N <= 0) {
        warnings.push_back("N was <= 0. Fixed to 1.");
        N = 1;
    }

    if (topology == BoardTopology::Finite) {
        if (width <= 0) {
            warnings.push_back("width was <= 0. Fixed to 1.");
            width = 1;
        }
        if (height <= 0) {
            warnings.push_back("height was <= 0. Fixed to 1.");
            height = 1;
        }
    } else {
        // Infinite ignores width/height as bounds
        if (width < 0) width = 0;
        if (height < 0) height = 0;
    }

    if (maxMoves < 0) {
        warnings.push_back("maxMoves was < 0. Fixed to 0 (unlimited).");
        maxMoves = 0;
    }

    if (!weightsEnabled && targetScore > 0) {
        warnings.push_back("targetScore is set but weightsEnabled=false. targetScore will be ignored.");
    }

    if (moveCostsEnabled) {
        if (costMode == CostMode::CostFromBudget && initialBudget < 0) {
            warnings.push_back("initialBudget < 0: treated as unlimited budget.");
        }
        if (costFunction.type == CellValueFunction::Type::Constant && costFunction.constant < 0) {
            warnings.push_back("costFunction.constant < 0: clamped to 0.");
            costFunction.constant = 0;
        }
    }

    if (weightsEnabled) {
        if (weightFunction.type == CellValueFunction::Type::Constant && weightFunction.constant < 0) {
            warnings.push_back("weightFunction.constant < 0: clamped to 0.");
            weightFunction.constant = 0;
        }
    }

    if (classicWin && maximizeLines) {
        warnings.push_back("classicWin=true and maximizeLines=true: classic win has priority (immediate game end).");
    }

    return warnings;
}

std::string toString(BoardTopology t) {
    switch (t) {
        case BoardTopology::Finite: return "finite";
        case BoardTopology::Infinite: return "infinite";
        default: return "unknown";
    }
}

std::string toString(LineLengthMode m) {
    switch (m) {
        case LineLengthMode::ExactN: return "exact";
        case LineLengthMode::AtLeastN: return "atleast";
        default: return "unknown";
    }
}

std::string toString(CostMode m) {
    switch (m) {
        case CostMode::CostFromScore: return "score";
        case CostMode::CostFromBudget: return "budget";
        default: return "unknown";
    }
}

}