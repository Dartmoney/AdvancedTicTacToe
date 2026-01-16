#ifndef TIKTAKTOE_COORD_H
#define TIKTAKTOE_COORD_H

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <ostream>

namespace engine {


class Coord final {
public:
    int x{0};
    int y{0};

    constexpr Coord() noexcept = default;
    constexpr Coord(int x_, int y_) noexcept : x(x_), y(y_) {}

    // Перегрузки операторов как методы класса (C++20)
    [[nodiscard]] constexpr Coord operator+(const Coord& rhs) const noexcept {
        return Coord{x + rhs.x, y + rhs.y};
    }

    [[nodiscard]] constexpr Coord operator-(const Coord& rhs) const noexcept {
        return Coord{x - rhs.x, y - rhs.y};
    }

    constexpr Coord& operator+=(const Coord& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Coord& operator-=(const Coord& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    // default-сравнение: сравнивает x, затем y
    [[nodiscard]] constexpr bool operator==(const Coord& rhs) const noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const Coord& rhs) const noexcept = default;
};

inline std::ostream& operator<<(std::ostream& os, const Coord& c) {
    os << "(" << c.x << "," << c.y << ")";
    return os;
}

// splitmix64 хорош как быстрый и стабильный миксер для двух int.
class CoordHash final {
public:
    constexpr std::size_t operator()(const Coord& c) const noexcept {
        std::uint64_t x = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.x));
        std::uint64_t y = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.y));
        std::uint64_t z = (x << 32) ^ y;

        // splitmix64
        z += 0x9e3779b97f4a7c15ull;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        z = z ^ (z >> 31);

        return static_cast<std::size_t>(z);
    }
};

} // namespace engine

namespace std {
template<>
struct hash<engine::Coord> {
    constexpr std::size_t operator()(const engine::Coord& c) const noexcept {
        return engine::CoordHash{}(c);
    }
};
} // namespace std



#endif