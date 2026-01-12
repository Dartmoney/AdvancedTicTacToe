#ifndef TIKTAKTOE_MOVE_H
#define TIKTAKTOE_MOVE_H
#pragma once

#include "Coord.h"
#include "Player.h"

namespace engine {

    // Move теперь class, и конструкторы сделаны именно перегрузками,
    // а не default-аргументом (как ты и попросил).
    class Move final {
    public:
        Coord coord{};
        Player player{Player::None};
        int cost{0};

        constexpr Move() noexcept = default;

        // Перегрузка: без стоимости
        constexpr Move(Coord c, Player p) noexcept
            : coord(c), player(p), cost(0) {}

        // Перегрузка: со стоимостью
        constexpr Move(Coord c, Player p, int cost_) noexcept
            : coord(c), player(p), cost(cost_) {}
    };

} // namespace engine


#endif