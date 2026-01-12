#ifndef TIKTAKTOE_AI_H
#define TIKTAKTOE_AI_H


#include <optional>
#include <random>

#include "GameState.h"

namespace engine {

    // Лёгкий, но заметно более сильный AI:
    // 1) Для классики 3x3 (N=3) — включает идеальную игру (minimax), чтобы как минимум играть вничью.
    // 2) Для всех остальных случаев — двухходовый (2-ply) поиск по ограниченному набору кандидатов
    //    + эвристики, которые по-разному работают в классике и в "игре на очки/линии".
    class SimpleAI {
    public:
        struct Settings {
            // Радиус генерации кандидатов вокруг занятых клеток
            int candidateRadius = 2;

            // Максимум кандидатов на 1 уровень (после генерации и сортировки)
            std::size_t maxCandidates = 600;

            // Сколько лучших ходов AI рассматривать глубже (2-ply).
            // 0 => рассматривать все (может быть тяжелее)
            std::size_t maxTopMoves = 40;

            // Сколько лучших ответов противника рассматривать на втором пли (2-ply).
            // 0 => рассматривать все
            std::size_t maxOpponentReplies = 40;

            // Включить 2-ply (ход AI → лучший ответ противника)
            bool enableTwoPly = true;

            // Включить идеальную игру (minimax) только для классики 3x3 (N=3)
            bool enablePerfectClassic3x3 = true;

            // 0 => random_device
            unsigned seed = 0;
        };

        explicit SimpleAI(Settings s = {});

        // Выбирает координату хода для aiPlayer.
        // Не меняет GameState (внутри делает копию доски для симуляции).
        std::optional<Coord> chooseMove(const GameState& state, Player aiPlayer);

    private:
        Settings s_;
        std::mt19937 rng_;
    };

} // namespace engine

#endif