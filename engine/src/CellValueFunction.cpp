#include "../include/engine/CellValueFunction.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace engine {

int CellValueFunction::value(Coord c) const noexcept {
    long long result = 0;

    switch (type) {
        case Type::Constant:
            result = constant;
            break;

        case Type::Manhattan: {
            long long dx = static_cast<long long>(c.x) - originX;
            long long dy = static_cast<long long>(c.y) - originY;
            result = offset + static_cast<long long>(scale) * (std::llabs(dx) + std::llabs(dy));
            break;
        }

        case Type::Chebyshev: {
            long long dx = static_cast<long long>(c.x) - originX;
            long long dy = static_cast<long long>(c.y) - originY;
            result = offset + static_cast<long long>(scale) * std::max(std::llabs(dx), std::llabs(dy));
            break;
        }

        case Type::RadialSquared: {
            long long dx = static_cast<long long>(c.x) - originX;
            long long dy = static_cast<long long>(c.y) - originY;
            result = offset + static_cast<long long>(scale) * (dx * dx + dy * dy);
            break;
        }

        case Type::Table: {
            if (tableWidth <= 0 || tableHeight <= 0) {
                result = defaultValue;
                break;
            }
            const int localX = c.x - tableOffsetX;
            const int localY = c.y - tableOffsetY;
            if (localX < 0 || localY < 0 || localX >= tableWidth || localY >= tableHeight) {
                result = defaultValue;
                break;
            }
            const std::size_t idx = static_cast<std::size_t>(localY) * static_cast<std::size_t>(tableWidth)
                                  + static_cast<std::size_t>(localX);
            if (idx >= table.size()) {
                result = defaultValue;
                break;
            }
            result = table[idx];
            break;
        }

        default:
            result = constant;
            break;
    }

    // Clamp to non-negative and int-range
    if (result < 0) result = 0;
    if (result > static_cast<long long>(std::numeric_limits<int>::max())) {
        result = static_cast<long long>(std::numeric_limits<int>::max());
    }

    return static_cast<int>(result);
}

CellValueFunction CellValueFunction::constantFunc(int v) {
    CellValueFunction f;
    f.type = Type::Constant;
    f.constant = v;
    return f;
}

std::string toString(CellValueFunction::Type t) {
    switch (t) {
        case CellValueFunction::Type::Constant: return "constant";
        case CellValueFunction::Type::Manhattan: return "manhattan";
        case CellValueFunction::Type::Chebyshev: return "chebyshev";
        case CellValueFunction::Type::RadialSquared: return "radialSquared";
        case CellValueFunction::Type::Table: return "table";
        default: return "unknown";
    }
}

} // namespace engine
