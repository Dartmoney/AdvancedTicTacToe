#include <engine/CellValueFunction.h>
#include <engine/GameState.h>

#include <iostream>

#define CHECK(cond)                                                                 \
    do {                                                                            \
        if (!(cond)) {                                                              \
            std::cerr << "CHECK failed at " << __FILE__ << ":" << __LINE__          \
                      << " : " << #cond << std::endl;                               \
            return 1;                                                               \
        }                                                                           \
    } while (0)

int main() {
    using namespace engine;

    // 1) AtLeastN, CountSubsegments=false: simple line count + undo
    {
        RuleSet rules;
        rules.topology = BoardTopology::Finite;
        rules.width = 5;
        rules.height = 5;
        rules.N = 3;
        rules.classicWin = false;
        rules.maximizeLines = true;
        rules.lineMode = LineLengthMode::AtLeastN;
        rules.countSubsegments = false;
        rules.weightsEnabled = false;
        rules.moveCostsEnabled = false;

        GameState g(rules, GameState::createBoard(rules));

        CHECK(g.tryMakeMove({0, 0}).ok); // X
        CHECK(g.tryMakeMove({0, 1}).ok); // O
        CHECK(g.tryMakeMove({1, 0}).ok); // X
        CHECK(g.tryMakeMove({0, 2}).ok); // O
        CHECK(g.tryMakeMove({2, 0}).ok); // X makes 3-in-row

        CHECK(g.stats(Player::X).lines == 1);
        CHECK(g.stats(Player::O).lines == 0);

        CHECK(g.undo());
        CHECK(g.board().get({2, 0}) == Player::None);
        CHECK(g.stats(Player::X).lines == 0);
    }

    // 2) AtLeastN, CountSubsegments=true: run length 4 gives 2 subsegments
    {
        RuleSet rules;
        rules.topology = BoardTopology::Finite;
        rules.width = 6;
        rules.height = 3;
        rules.N = 3;
        rules.classicWin = false;
        rules.maximizeLines = true;
        rules.lineMode = LineLengthMode::AtLeastN;
        rules.countSubsegments = true;

        GameState g(rules, GameState::createBoard(rules));

        CHECK(g.tryMakeMove({0, 0}).ok); // X
        CHECK(g.tryMakeMove({0, 1}).ok); // O
        CHECK(g.tryMakeMove({1, 0}).ok); // X
        CHECK(g.tryMakeMove({1, 1}).ok); // O
        CHECK(g.tryMakeMove({2, 0}).ok); // X => one segment
        CHECK(g.stats(Player::X).lines == 1);

        CHECK(g.tryMakeMove({2, 1}).ok); // O
        CHECK(g.tryMakeMove({3, 0}).ok); // X => run 4 -> 2 segments
        CHECK(g.stats(Player::X).lines == 2);
    }

    // 3) Weights enabled: constant weight=1, AtLeastN, no subsegments
    {
        RuleSet rules;
        rules.topology = BoardTopology::Finite;
        rules.width = 5;
        rules.height = 3;
        rules.N = 3;
        rules.classicWin = false;
        rules.maximizeLines = false;
        rules.lineMode = LineLengthMode::AtLeastN;
        rules.countSubsegments = false;
        rules.weightsEnabled = true;
        rules.weightFunction = CellValueFunction::constantFunc(1);

        GameState g(rules, GameState::createBoard(rules));

        CHECK(g.tryMakeMove({0, 0}).ok); // X
        CHECK(g.tryMakeMove({0, 1}).ok); // O
        CHECK(g.tryMakeMove({1, 0}).ok); // X
        CHECK(g.tryMakeMove({1, 1}).ok); // O
        CHECK(g.tryMakeMove({2, 0}).ok); // X forms length=3 => sum=3, score=3*3=9
        CHECK(g.stats(Player::X).score == 9);
    }

    // 4) Budget costs: cost=2, budget=3 => X can do 1 move then stuck
    {
        RuleSet rules;
        rules.topology = BoardTopology::Finite;
        rules.width = 5;
        rules.height = 5;
        rules.N = 3;
        rules.classicWin = false;
        rules.maximizeLines = false;

        rules.weightsEnabled = true;
        rules.weightFunction = CellValueFunction::constantFunc(1);

        rules.moveCostsEnabled = true;
        rules.costFunction = CellValueFunction::constantFunc(2);
        rules.costMode = CostMode::CostFromBudget;
        rules.initialBudget = 3;

        GameState g(rules, GameState::createBoard(rules));

        CHECK(g.tryMakeMove({0, 0}).ok); // X cost 2 => budget 1
        CHECK(g.stats(Player::X).budget == 1);

        CHECK(g.tryMakeMove({0, 1}).ok); // O
        auto res = g.tryMakeMove({1, 0}); // X cost 2 => fail
        CHECK(!res.ok);
    }

    std::cout << "All tests passed.\n";
    return 0;
}
