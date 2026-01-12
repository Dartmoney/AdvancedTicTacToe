#ifndef TIKTAKTOE_COORD_H
#define TIKTAKTOE_COORD_H
#pragma once

#include <cstdint>
#include <cstdlib>
#include <ostream>

namespace engine {

    struct Coord {
        int x = 0;
        int y = 0;

        constexpr Coord() noexcept = default;
        constexpr Coord(int x_, int y_) noexcept : x(x_), y(y_) {}
    };

    constexpr bool operator==(const Coord& a, const Coord& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
    constexpr bool operator!=(const Coord& a, const Coord& b) noexcept {
        return !(a == b);
    }

    constexpr Coord operator+(const Coord& a, const Coord& b) noexcept {
        return Coord{a.x + b.x, a.y + b.y};
    }
    constexpr Coord operator-(const Coord& a, const Coord& b) noexcept {
        return Coord{a.x - b.x, a.y - b.y};
    }

    inline std::ostream& operator<<(std::ostream& os, const Coord& c) {
        os << "(" << c.x << "," << c.y << ")";
        return os;
    }

    struct CoordHash {
        std::size_t operator()(const Coord& c) const noexcept {
            std::uint64_t x = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.x));
            std::uint64_t y = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.y));
            std::uint64_t z = (x << 32) ^ y;

            z += 0x9e3779b97f4a7c15ull;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            z = z ^ (z >> 31);

            return static_cast<std::size_t>(z);
        }
    };

}

namespace std {
    template<>
    struct hash<engine::Coord> {
        std::size_t operator()(const engine::Coord& c) const noexcept {
            return engine::CoordHash{}(c);
        }
    };
}

#endif