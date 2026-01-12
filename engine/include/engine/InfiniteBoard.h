#ifndef TIKTAKTOE_INFINITEBOARD_H
#define TIKTAKTOE_INFINITEBOARD_H
#pragma once

#include <unordered_map>

#include "Board.h"

namespace engine {

    class InfiniteBoard final : public IBoard {
    public:
        InfiniteBoard() = default;

        bool isFinite() const noexcept override { return false; }
        int width() const noexcept override { return 0; }
        int height() const noexcept override { return 0; }
        bool inBounds(Coord) const noexcept override { return true; }

        Player get(Coord c) const override;
        bool set(Coord c, Player p) override;
        bool clear(Coord c) override;

        std::vector<std::pair<Coord, Player>> occupied() const override;

        std::size_t occupiedCount() const noexcept { return cells_.size(); }

    private:
        std::unordered_map<Coord, Player, CoordHash> cells_;
    };

}

#endif