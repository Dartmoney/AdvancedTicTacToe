#ifndef TIKTAKTOE_MOVE_H
#define TIKTAKTOE_MOVE_H
#pragma once
#include "Coord.h"
#include "Player.h"

namespace engine {

    // Был struct — сделали class. Оставили public-поля, чтобы не ломать код.
    // Добавили constexpr конструктор для удобства и корректной "constexpr-совместимости".
    class Move {
    public:
        Coord coord{};
        Player player = Player::None;
        int cost = 0;

        constexpr Move() noexcept = default;
        constexpr Move(Coord c, Player p, int cost_ = 0) noexcept : coord(c), player(p), cost(cost_) {}
    };

} // namespace engine

#endif