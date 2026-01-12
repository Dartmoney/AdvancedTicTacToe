//
// Created by imako on 08.01.2026.
//

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
        virtual int width() const noexcept = 0;   // for finite boards
        virtual int height() const noexcept = 0;  // for finite boards
        virtual bool inBounds(Coord c) const noexcept = 0;

        virtual Player get(Coord c) const = 0;
        virtual bool set(Coord c, Player p) = 0;  // false if invalid/occupied when p!=None
        virtual bool clear(Coord c) = 0;

        virtual std::vector<std::pair<Coord, Player>> occupied() const = 0;

        bool isEmpty(Coord c) const { return get(c) == Player::None; }
    };

} // namespace engine

#endif //TIKTAKTOE_BOARD_H