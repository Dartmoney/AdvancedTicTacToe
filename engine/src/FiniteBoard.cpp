#include "../include/engine/FiniteBoard.h"

#include <stdexcept>

namespace engine {

    FiniteBoard::FiniteBoard(int w, int h) : width_(w), height_(h) {
        if (w <= 0 || h <= 0) {
            throw std::invalid_argument("FiniteBoard: width/height must be > 0");
        }
        cells_.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), Player::None);
    }

    bool FiniteBoard::inBounds(Coord c) const noexcept {
        return (c.x >= 0 && c.y >= 0 && c.x < width_ && c.y < height_);
    }

    Player FiniteBoard::get(Coord c) const {
        if (!inBounds(c)) {
            return Player::None;
        }
        return cells_[idx(c)];
    }

    bool FiniteBoard::set(Coord c, Player p) {
        if (p == Player::None) {
            return clear(c);
        }
        if (!inBounds(c)) {
            return false;
        }
        auto& cell = cells_[idx(c)];
        if (cell != Player::None) {
            return false;
        }
        cell = p;
        return true;
    }

    bool FiniteBoard::clear(Coord c) {
        if (!inBounds(c)) {
            return false;
        }
        auto& cell = cells_[idx(c)];
        if (cell == Player::None) {
            return false;
        }
        cell = Player::None;
        return true;
    }

    std::vector<std::pair<Coord, Player>> FiniteBoard::occupied() const {
        std::vector<std::pair<Coord, Player>> out;
        out.reserve(cells_.size());
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                Player p = cells_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
                if (p != Player::None) {
                    out.push_back({Coord{x, y}, p});
                }
            }
        }
        return out;
    }

} // namespace engine
