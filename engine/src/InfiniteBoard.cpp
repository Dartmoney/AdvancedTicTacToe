#include "../include/engine/InfiniteBoard.h"

namespace engine {

    Player InfiniteBoard::get(Coord c) const {
        auto it = cells_.find(c);
        if (it == cells_.end()) {
            return Player::None;
        }
        return it->second;
    }

    bool InfiniteBoard::set(Coord c, Player p) {
        if (p == Player::None) {
            return clear(c);
        }
        auto [it, inserted] = cells_.emplace(c, p);
        if (!inserted) {
            return false;
        }
        return true;
    }

    bool InfiniteBoard::clear(Coord c) {
        return cells_.erase(c) > 0;
    }

    std::vector<std::pair<Coord, Player>> InfiniteBoard::occupied() const {
        std::vector<std::pair<Coord, Player>> out;
        out.reserve(cells_.size());
        for (const auto& kv : cells_) {
            out.push_back({kv.first, kv.second});
        }
        return out;
    }

}

