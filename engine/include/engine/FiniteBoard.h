//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_FINITEBOARD_H
#define TIKTAKTOE_FINITEBOARD_H
#pragma once

#include <vector>

#include "Board.h"

namespace engine {

    class FiniteBoard final : public IBoard {
    public:
        FiniteBoard(int w, int h);

        bool isFinite() const noexcept override { return true; }
        int width() const noexcept override { return width_; }
        int height() const noexcept override { return height_; }
        bool inBounds(Coord c) const noexcept override;

        Player get(Coord c) const override;
        bool set(Coord c, Player p) override;
        bool clear(Coord c) override;

        std::vector<std::pair<Coord, Player>> occupied() const override;

    private:
        int width_ = 0;
        int height_ = 0;
        std::vector<Player> cells_; // row-major y*width + x

        std::size_t idx(Coord c) const noexcept {
            return static_cast<std::size_t>(c.y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(c.x);
        }
    };

} // namespace engine

#endif //TIKTAKTOE_FINITEBOARD_H