#include "../include/engine/AI.h"

#include "../include/engine/Scoring.h"

#include <limits>
#include <random>
#include "engine/AI.h"

#include "engine/FiniteBoard.h"
#include "engine/InfiniteBoard.h"
#include "engine/Scoring.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

namespace engine {
namespace {

// ─────────────────────────────────────────────────────────────────────────────
// Утилиты и "внутренние" структуры симуляции
// ─────────────────────────────────────────────────────────────────────────────

constexpr long long NEG_INF = (std::numeric_limits<long long>::min() / 4);

struct SimPlayerState {
    long long score = 0;
    int lines = 0;
    long long budget = 0; // <0 => бесконечный бюджет (как в движке)
};

SimPlayerState simStatsFrom(const GameState& state, Player p) {
    const auto& s = state.stats(p);
    SimPlayerState out;
    out.score = s.score;
    out.lines = s.lines;
    out.budget = s.budget;
    return out;
}

int costAt(const RuleSet& rules, Coord c) {
    if (!rules.moveCostsEnabled) return 0;
    int v = rules.costFunction.value(c);
    if (v < 0) v = 0;
    return v;
}

bool isMoveLegalForPlayer(const IBoard& board, const RuleSet& rules, Player /*p*/, Coord c, long long budget) {
    if (board.isFinite() && !board.inBounds(c)) return false;
    if (board.get(c) != Player::None) return false;

    if (rules.moveCostsEnabled && rules.costMode == CostMode::CostFromBudget) {
        const int cost = costAt(rules, c);
        if (budget >= 0 && budget < cost) return false;
    }
    return true;
}

long long manhattan(Coord a, Coord b) {
    return std::llabs(static_cast<long long>(a.x) - b.x) + std::llabs(static_cast<long long>(a.y) - b.y);
}

Coord defaultRef(const IBoard& board) {
    if (board.isFinite()) {
        return Coord{board.width() / 2, board.height() / 2};
    }
    return Coord{0, 0};
}

std::unique_ptr<IBoard> cloneBoard(const IBoard& src) {
    std::unique_ptr<IBoard> dst;

    if (src.isFinite()) {
        dst = std::make_unique<FiniteBoard>(src.width(), src.height());
    } else {
        dst = std::make_unique<InfiniteBoard>();
    }

    for (const auto& [c, p] : src.occupied()) {
        (void)dst->set(c, p);
    }
    return dst;
}

// Генерация кандидатов вокруг занятых клеток (аналогично GameState::generateCandidateMoves,
// но работает по любому IBoard и для симуляций).
std::vector<Coord> neighborhoodCandidates(const IBoard& board, Coord ref, int radius, std::size_t maxCandidates) {
    std::vector<Coord> out;
    if (radius < 0) radius = 0;

    const auto occ = board.occupied();
    if (occ.empty()) {
        out.push_back(defaultRef(board));
        return out;
    }

    std::unordered_set<Coord, CoordHash> set;
    set.reserve(std::min<std::size_t>(maxCandidates > 0 ? maxCandidates * 2 : 2048, 8192));

    for (const auto& [cell, p] : occ) {
        (void)p;
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                Coord c{cell.x + dx, cell.y + dy};
                if (board.isFinite() && !board.inBounds(c)) continue;
                if (board.get(c) != Player::None) continue;
                set.insert(c);
            }
        }
    }

    out.assign(set.begin(), set.end());

    std::sort(out.begin(), out.end(), [&](const Coord& a, const Coord& b) {
        const long long da = manhattan(a, ref);
        const long long db = manhattan(b, ref);
        if (da != db) return da < db;
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    });

    if (maxCandidates > 0 && out.size() > maxCandidates) {
        out.resize(maxCandidates);
    }
    return out;
}

std::vector<Coord> allEmptyFinite(const IBoard& board) {
    std::vector<Coord> out;
    if (!board.isFinite()) return out;

    out.reserve(static_cast<std::size_t>(board.width()) * static_cast<std::size_t>(board.height()));
    for (int y = 0; y < board.height(); ++y) {
        for (int x = 0; x < board.width(); ++x) {
            Coord c{x, y};
            if (board.get(c) == Player::None) out.push_back(c);
        }
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Оценка "потенциала линии" для классики/угроз (без полного minimax)
// ─────────────────────────────────────────────────────────────────────────────

struct LinePotential {
    bool canWin = false;       // если поставить сюда — получится линия >= N
    int bestLen = 1;           // максимальная длина после постановки
    int bestOpenEnds = 0;      // 0/1/2 открытых конца у лучшей линии
    long long value = 0;       // суммарная ценность угроз по 4 направлениям
};

// Важно: функция рассчитана под "классическую" угрозу (континьюс-отрезки).
// Она не учитывает "дырки" вида XX_XX. Это компенсируется 2-ply + тем, что на практике
// такие угрозы всё равно проявляются через лучшие ответы.
long long directionValue(int len, int openEnds, int N) {
    if (len <= 0) return 0;
    if (len >= N) return 1'000'000'000'000LL;

    // База растёт полиномиально, чтобы не взрываться при больших N.
    const long long l = static_cast<long long>(len);
    const long long base = l * l * l * l; // l^4

    const long long oe = (openEnds == 2 ? 10 : (openEnds == 1 ? 3 : 1));

    long long bonus = 1;
    if (N >= 2) {
        if (len == N - 1) bonus = 2000;
        else if (len == N - 2) bonus = 200;
        else if (len == N - 3) bonus = 40;
    }
    return base * oe * bonus;
}

LinePotential computePotentialAtEmptyCell(const IBoard& board, const RuleSet& rules, Player p, Coord c) {
    LinePotential pot;
    const int N = std::max(1, rules.N);

    struct Dir { int dx; int dy; };
    const Dir dirs[4] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (const auto& d : dirs) {
        int left = 0;
        Coord t{c.x - d.dx, c.y - d.dy};
        while (board.inBounds(t) && board.get(t) == p) {
            ++left;
            t.x -= d.dx;
            t.y -= d.dy;
        }

        int right = 0;
        t = Coord{c.x + d.dx, c.y + d.dy};
        while (board.inBounds(t) && board.get(t) == p) {
            ++right;
            t.x += d.dx;
            t.y += d.dy;
        }

        const int len = left + 1 + right;

        Coord leftEnd{c.x - (left + 1) * d.dx, c.y - (left + 1) * d.dy};
        Coord rightEnd{c.x + (right + 1) * d.dx, c.y + (right + 1) * d.dy};

        int openEnds = 0;
        if (board.inBounds(leftEnd) && board.get(leftEnd) == Player::None) ++openEnds;
        if (board.inBounds(rightEnd) && board.get(rightEnd) == Player::None) ++openEnds;

        if (len >= N) pot.canWin = true;

        if (len > pot.bestLen) {
            pot.bestLen = len;
            pot.bestOpenEnds = openEnds;
        } else if (len == pot.bestLen && openEnds > pot.bestOpenEnds) {
            pot.bestOpenEnds = openEnds;
        }

        pot.value += directionValue(len, openEnds, N);
    }

    return pot;
}

// ─────────────────────────────────────────────────────────────────────────────
// Режимы оценки: Classic vs ScoreLike
// ─────────────────────────────────────────────────────────────────────────────

enum class AiMode { Classic, ScoreLike };

AiMode selectMode(const RuleSet& rules) {
    if (rules.classicWin) return AiMode::Classic;
    if (rules.weightsEnabled || rules.maximizeLines) return AiMode::ScoreLike;
    // если никакие цели не включены — всё равно играем "как классика" (угрозами)
    return AiMode::Classic;
}

long long budgetPenalty(const RuleSet& rules, long long budget, int cost, long long baseMul) {
    if (!rules.moveCostsEnabled || rules.costMode != CostMode::CostFromBudget) return 0;
    if (cost <= 0) return 0;

    long long p = static_cast<long long>(cost) * baseMul;

    // Если бюджет конечный и стоимость "съедает" заметную долю — штрафуем сильнее.
    if (budget >= 0) {
        const long long ratio = (static_cast<long long>(cost) * 100) / (budget + 1); // 0..100+
        p += (p * ratio) / 100; // до ~2x и больше, если cost>budget (но такой ход отфильтруем как illegal)
    }
    return p;
}

long long distancePenalty(const IBoard& board, Coord c, Coord ref, long long mul) {
    // На finite и infinite одинаково: AI любит держаться около активной зоны.
    // Если хочется "центровую" стратегию — ref можно делать центром поля.
    (void)board;
    return manhattan(c, ref) * mul;
}

long long evalClassicMove(const IBoard& board,
                          const RuleSet& rules,
                          Player p,
                          Coord c,
                          const SimPlayerState& self,
                          Coord ref) {
    if (!isMoveLegalForPlayer(board, rules, p, c, self.budget)) return NEG_INF;

    const int cost = costAt(rules, c);

    // Своё усиление и блок угроз противника считаем отдельно.
    const LinePotential myPot = computePotentialAtEmptyCell(board, rules, p, c);
    const LinePotential opPot = computePotentialAtEmptyCell(board, rules, other(p), c);

    // Большие константы подобраны так, чтобы:
    // - "выиграть сейчас" > "всё остальное"
    // - "заблокировать мгновенный проигрыш" тоже очень важно
    constexpr long long WIN_NOW   = 9'000'000'000'000LL;
    constexpr long long BLOCK_NOW = 8'000'000'000'000LL;

    long long s = 0;

    if (myPot.canWin) s += WIN_NOW;
    if (opPot.canWin) s += BLOCK_NOW;

    // Угрозы: свой потенциал обычно важнее, но блокирование тоже приоритетно.
    s += myPot.value * 80;
    s += opPot.value * 60;

    // Небольшое предпочтение держаться ближе к "центру действия"
    s -= distancePenalty(board, c, ref, 2'000);

    // Стоимость хода (бюджет): в классике штрафуем заметно, но не сильнее чем блок/выигрыш.
    s -= budgetPenalty(rules, self.budget, cost, 200'000);

    return s;
}

long long evalScoreMove(const IBoard& board,
                        const RuleSet& rules,
                        Player p,
                        Coord c,
                        const SimPlayerState& self,
                        Coord ref) {
    if (!isMoveLegalForPlayer(board, rules, p, c, self.budget)) return NEG_INF;

    const int cost = costAt(rules, c);

    // Инкрементальная оценка "прибавки" по правилам линий/веса.
    const MoveDelta d = Scoring::computeMoveDelta(board, c, p, rules);

    // Если одновременно включена classicWin — она имеет приоритет. Значит, AI должен это уважать.
    if (rules.classicWin && d.maxRunLen >= rules.N) {
        return 9'000'000'000'000LL; // "выиграть прямо сейчас"
    }

    // Эффективная прибавка очков:
    // - если costMode==CostFromScore, стоимость уменьшает score → учитываем сразу
    long long effScoreDelta = d.scoreDelta;
    if (rules.moveCostsEnabled && rules.costMode == CostMode::CostFromScore) {
        effScoreDelta -= cost;
    }

    // TargetScore — мгновенная победа по очкам
    if (rules.weightsEnabled && rules.targetScore > 0) {
        const long long newScore = self.score + effScoreDelta;
        if (newScore >= rules.targetScore) {
            return 8'500'000'000'000LL;
        }
    }

    long long s = 0;

    // Линии (если maximizeLines включён, линии — основная цель партии)
    const long long lineW = rules.maximizeLines ? 1'000'000LL : 300'000LL;
    s += static_cast<long long>(d.linesDelta) * lineW;

    // Очки (weightsEnabled). Масштабируем, чтобы сопоставить с "линиями".
    if (rules.weightsEnabled) {
        const long long scoreW = (rules.targetScore > 0 ? 50'000LL : 12'000LL);
        s += effScoreDelta * scoreW;
    }

    // Небольшой "классический" компонент: помогает строить будущие линии даже если сейчас delta=0.
    // (Особенно полезно при больших N)
    const LinePotential myPot = computePotentialAtEmptyCell(board, rules, p, c);
    const LinePotential opPot = computePotentialAtEmptyCell(board, rules, other(p), c);

    s += myPot.value * 2;
    s += opPot.value * 1; // чуть-чуть за блокировку будущих угроз

    // Стоимость хода при бюджетном режиме: в score-режиме штрафуем мягче, чем в классике.
    // (В "игре на очки" иногда выгодно заплатить дороже ради большого gain)
    s -= budgetPenalty(rules, self.budget, cost, 40'000);

    // Держаться ближе к активной зоне
    s -= distancePenalty(board, c, ref, 300);

    return s;
}

// Унифицированная оценка (в зависимости от режима)
long long evalMoveByMode(const IBoard& board,
                         const RuleSet& rules,
                         AiMode mode,
                         Player p,
                         Coord c,
                         const SimPlayerState& self,
                         Coord ref) {
    if (mode == AiMode::Classic) {
        return evalClassicMove(board, rules, p, c, self, ref);
    }
    return evalScoreMove(board, rules, p, c, self, ref);
}

// ─────────────────────────────────────────────────────────────────────────────
// Идеальная классика 3x3: minimax (только для N=3, без очков/стоимости)
// ─────────────────────────────────────────────────────────────────────────────

Player winner3x3(const std::array<Player, 9>& b) {
    const int lines[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };

    for (const auto& ln : lines) {
        const Player p = b[ln[0]];
        if (p != Player::None && p == b[ln[1]] && p == b[ln[2]]) {
            return p;
        }
    }
    return Player::None;
}

bool boardFull3x3(const std::array<Player, 9>& b) {
    return std::none_of(b.begin(), b.end(), [](Player p) { return p == Player::None; });
}

int minimax3x3(std::array<Player, 9>& b, Player toMove, Player aiPlayer, int depth) {
    const Player w = winner3x3(b);
    if (w == aiPlayer) return 10 - depth;
    if (w == other(aiPlayer)) return depth - 10;
    if (boardFull3x3(b)) return 0;

    if (toMove == aiPlayer) {
        int best = -1000;
        for (int i = 0; i < 9; ++i) {
            if (b[i] != Player::None) continue;
            b[i] = toMove;
            best = std::max(best, minimax3x3(b, other(toMove), aiPlayer, depth + 1));
            b[i] = Player::None;
        }
        return best;
    } else {
        int best = 1000;
        for (int i = 0; i < 9; ++i) {
            if (b[i] != Player::None) continue;
            b[i] = toMove;
            best = std::min(best, minimax3x3(b, other(toMove), aiPlayer, depth + 1));
            b[i] = Player::None;
        }
        return best;
    }
}

std::optional<Coord> choosePerfectClassic3x3(const GameState& state, Player aiPlayer) {
    std::array<Player, 9> b{};
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            b[y * 3 + x] = state.board().get(Coord{x, y});
        }
    }

    int bestScore = -1000;
    std::optional<int> bestIdx;

    // Лёгкий приоритет центра при равенстве
    const int preferOrder[9] = {4, 0,2,6,8, 1,3,5,7};

    for (int k = 0; k < 9; ++k) {
        const int i = preferOrder[k];
        if (b[i] != Player::None) continue;
        b[i] = aiPlayer;
        int s = minimax3x3(b, other(aiPlayer), aiPlayer, 1);
        b[i] = Player::None;

        if (!bestIdx || s > bestScore) {
            bestScore = s;
            bestIdx = i;
        }
    }

    if (!bestIdx) return std::nullopt;
    const int idx = *bestIdx;
    return Coord{idx % 3, idx / 3};
}

bool isPerfect3x3Case(const GameState& state, const RuleSet& rules) {
    if (!state.board().isFinite()) return false;
    if (state.board().width() != 3 || state.board().height() != 3) return false;

    if (!rules.classicWin) return false;
    if (rules.N != 3) return false;

    // minimax реализован только для "чистой" классики без очков/стоимости,
    // иначе функция выигрыша меняется.
    if (rules.weightsEnabled) return false;
    if (rules.moveCostsEnabled) return false;

    return true;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Реализация SimpleAI
// ─────────────────────────────────────────────────────────────────────────────

SimpleAI::SimpleAI(Settings s) : s_(s) {
    if (s_.seed == 0) {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    } else {
        rng_ = std::mt19937(s_.seed);
    }
}

std::optional<Coord> SimpleAI::chooseMove(const GameState& state, Player aiPlayer) {
    if (state.isGameOver()) return std::nullopt;

    const RuleSet& rules = state.rules();
    const AiMode mode = selectMode(rules);

    // Идеальная игра на 3x3, чтобы AI не проигрывал и умел играть вничью.
    if (s_.enablePerfectClassic3x3 && isPerfect3x3Case(state, rules)) {
        return choosePerfectClassic3x3(state, aiPlayer);
    }

    // Копируем доску (чтобы безопасно симулировать ходы в 2-ply без модификации GameState)
    auto boardPtr = cloneBoard(state.board());
    IBoard& board = *boardPtr;

    const Player opp = other(aiPlayer);

    const SimPlayerState aiS = simStatsFrom(state, aiPlayer);
    const SimPlayerState opS = simStatsFrom(state, opp);

    // "Центр действия": если есть lastMove — тянем игру туда, иначе в центр поля/0,0
    Coord ref = state.lastMove().value_or(defaultRef(board));

    // 1) Сгенерировать кандидатов для AI
    std::vector<Coord> cand = neighborhoodCandidates(board, ref, s_.candidateRadius, s_.maxCandidates);

    // 2) Оставить только легальные
    std::vector<Coord> legal;
    legal.reserve(cand.size());
    for (const auto& c : cand) {
        if (isMoveLegalForPlayer(board, rules, aiPlayer, c, aiS.budget)) {
            legal.push_back(c);
        }
    }

    // 3) Fallback: если finite и вокруг занятых ничего не подошло (например, из-за бюджета) —
    //    просканируем всё поле.
    if (legal.empty() && board.isFinite()) {
        auto all = allEmptyFinite(board);
        Coord center = defaultRef(board);

        std::sort(all.begin(), all.end(), [&](const Coord& a, const Coord& b) {
            const long long da = manhattan(a, center);
            const long long db = manhattan(b, center);
            if (da != db) return da < db;
            if (a.y != b.y) return a.y < b.y;
            return a.x < b.x;
        });

        for (const auto& c : all) {
            if (isMoveLegalForPlayer(board, rules, aiPlayer, c, aiS.budget)) {
                legal.push_back(c);
            }
        }

        if (s_.maxCandidates > 0 && legal.size() > s_.maxCandidates) {
            legal.resize(s_.maxCandidates);
        }
    }

    // 4) Infinite fallback: если совсем нет — попробуем в ближней окрестности ref расширяясь
    if (legal.empty() && !board.isFinite()) {
        bool found = false;
        for (int r = 0; r <= 8 && !found; ++r) {
            for (int dy = -r; dy <= r && !found; ++dy) {
                for (int dx = -r; dx <= r && !found; ++dx) {
                    Coord c{ref.x + dx, ref.y + dy};
                    if (board.get(c) != Player::None) continue;
                    if (isMoveLegalForPlayer(board, rules, aiPlayer, c, aiS.budget)) {
                        legal.push_back(c);
                        found = true;
                    }
                }
            }
        }
    }

    if (legal.empty()) return std::nullopt;

    // 5) Быстрая оценка всех кандидатов
    struct ScoredMove {
        Coord c{};
        long long score = NEG_INF;
    };

    std::vector<ScoredMove> scored;
    scored.reserve(legal.size());

    for (const auto& c : legal) {
        const long long s = evalMoveByMode(board, rules, mode, aiPlayer, c, aiS, ref);
        scored.push_back({c, s});
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredMove& a, const ScoredMove& b) {
        return a.score > b.score;
    });

    // Ограничим число рассматриваемых "глубоких" ходов
    if (s_.maxTopMoves > 0 && scored.size() > s_.maxTopMoves) {
        scored.resize(s_.maxTopMoves);
    }

    // Небольшой шум для "не одинаковой" игры при равных оценках
    std::uniform_int_distribution<int> noise(0, 9999);

    long long bestFinal = NEG_INF;
    std::optional<Coord> best;

    // Если 2-ply отключён — просто берём лучший по эвристике
    if (!s_.enableTwoPly) {
        for (const auto& m : scored) {
            long long s = m.score + noise(rng_);
            if (!best || s > bestFinal) {
                bestFinal = s;
                best = m.c;
            }
        }
        return best;
    }

    // 6) 2-ply: для каждого хода AI смотрим лучший ответ противника
    for (const auto& m : scored) {
        const Coord myMove = m.c;

        // Симулируем постановку AI
        if (!board.set(myMove, aiPlayer)) continue; // на всякий случай
        // RAII-откат
        struct Guard {
            IBoard& b; Coord c;
            ~Guard() { b.clear(c); }
        } guard{board, myMove};

        // Сгенерируем ответы противника вокруг "последнего" хода
        std::vector<Coord> oppCand = neighborhoodCandidates(board, myMove, s_.candidateRadius, s_.maxCandidates);

        std::vector<ScoredMove> oppScored;
        oppScored.reserve(oppCand.size());

        for (const auto& oc : oppCand) {
            if (!isMoveLegalForPlayer(board, rules, opp, oc, opS.budget)) continue;
            const long long os = evalMoveByMode(board, rules, mode, opp, oc, opS, myMove);
            oppScored.push_back({oc, os});
        }

        if (oppScored.empty()) {
            // Противник не нашёл легальный ответ в нашей выборке.
            // (В реальности на infinite может существовать ход далеко, но мы сознательно ограничиваемся окрестностью.)
            // В таком случае просто считаем, что "ответ слабый".
            long long final = m.score + noise(rng_);
            if (!best || final > bestFinal) {
                bestFinal = final;
                best = myMove;
            }
            continue;
        }

        std::sort(oppScored.begin(), oppScored.end(), [](const ScoredMove& a, const ScoredMove& b) {
            return a.score > b.score;
        });

        if (s_.maxOpponentReplies > 0 && oppScored.size() > s_.maxOpponentReplies) {
            oppScored.resize(s_.maxOpponentReplies);
        }

        // Лучший ответ противника (с его точки зрения)
        const long long oppBest = oppScored.front().score;

        // Чем классичнее режим — тем сильнее мы боимся ответа противника (оборона важнее).
        const long long defenseMul = (mode == AiMode::Classic ? 2 : 1);

        long long final = m.score - oppBest * defenseMul;
        final += noise(rng_);

        if (!best || final > bestFinal) {
            bestFinal = final;
            best = myMove;
        }
    }

    return best;
}

} // namespace engine
