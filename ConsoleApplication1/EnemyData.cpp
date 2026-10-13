
// EnemyData.cpp
#include "EnemyData.h"

const std::vector<EnemyData>& EnemyDatabase::Table()
{
    static const std::vector<EnemyData> table = {
        //        名前        HP  攻撃 経験値 加入率 出現開始
        EnemyData("スライム", 20,  5, 10, 0.40, 1),
        EnemyData("ゴブリン", 35,  8, 20, 0.25, 1),
        EnemyData("ウルフ",   28, 10, 18, 0.30, 3),
        EnemyData("ゴーレム", 70, 15, 45, 0.10, 8),
    };
    return table;
}

const EnemyData& EnemyDatabase::PickRandom(std::mt19937& rng, int explorationCount)
{
    std::vector<const EnemyData*> candidates;
    for (const auto& d : Table())
        if (d.minExploration <= explorationCount) candidates.push_back(&d);
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return *candidates[dist(rng)];
}
