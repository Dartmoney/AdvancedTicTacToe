#ifndef TIKTAKTOE_BOARD_H
#define TIKTAKTOE_BOARD_H
#pragma once

#include <utility>
#include <vector>

#include "Coord.h"
#include "Player.h"

namespace engine {

    class IBoard {
    public:
        virtual ~IBoard() = default;

        virtual bool isFinite() const noexcept = 0;
        virtual int width() const noexcept = 0;
        virtual int height() const noexcept = 0;
        virtual bool inBounds(Coord c) const noexcept = 0;

        virtual Player get(Coord c) const = 0;
        virtual bool set(Coord c, Player p) = 0;
        virtual bool clear(Coord c) = 0;

        virtual std::vector<std::pair<Coord, Player>> occupied() const = 0;

        bool isEmpty(Coord c) const { return get(c) == Player::None; }
    };

}

#endif