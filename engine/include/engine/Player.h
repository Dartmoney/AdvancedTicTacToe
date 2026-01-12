//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_PLAYER_H
#define TIKTAKTOE_PLAYER_H
#pragma once

#include <cstdint>
#include <string_view>

namespace engine {

    enum class Player : std::uint8_t { None = 0, X = 1, O = 2 };

    constexpr Player other(Player p) noexcept {
        return (p == Player::X) ? Player::O : (p == Player::O ? Player::X : Player::None);
    }

    constexpr int playerIndex(Player p) noexcept {
        return (p == Player::X) ? 0 : (p == Player::O ? 1 : -1);
    }

    constexpr char toChar(Player p) noexcept {
        return (p == Player::X) ? 'X' : (p == Player::O ? 'O' : '.');
    }

    constexpr std::string_view toString(Player p) noexcept {
        switch (p) {
            case Player::X: return "X";
            case Player::O: return "O";
            default: return "None";
        }
    }

} // namespace engine

#endif //TIKTAKTOE_PLAYER_H