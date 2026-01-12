#ifndef TIKTAKTOE_MOVE_H
#define TIKTAKTOE_MOVE_H
#pragma once
#include "Coord.h"
#include "Player.h"

namespace engine {

    struct Move {
        Coord coord{};
        Player player = Player::None;
        int cost = 0;
    };

}
#endif