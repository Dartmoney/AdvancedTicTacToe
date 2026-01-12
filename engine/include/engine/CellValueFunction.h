#ifndef TIKTAKTOE_CELLVALUEFUNCTION_H
#define TIKTAKTOE_CELLVALUEFUNCTION_H
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Coord.h"

namespace engine {

    struct CellValueFunction {
        enum class Type : std::uint8_t { Constant = 0, Manhattan = 1, Chebyshev = 2, RadialSquared = 3, Table = 4 };

        Type type = Type::Constant;

        // Common params
        int constant = 0;
        int scale = 1;
        int offset = 0;
        int originX = 0;
        int originY = 0;

        // Table params
        int tableWidth = 0;
        int tableHeight = 0;
        int tableOffsetX = 0; // world coord that maps to table x=0
        int tableOffsetY = 0; // world coord that maps to table y=0
        int defaultValue = 0;
        std::vector<int> table; // size = tableWidth*tableHeight

        int value(Coord c) const noexcept;

        static CellValueFunction constantFunc(int v);
    };

    std::string toString(CellValueFunction::Type t);

}

#endif