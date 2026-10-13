// Factory.cpp
#include "Factory.h"

std::unique_ptr<Enemy> EnemyFactory::Create(const EnemyData& data, int explorationCount)
{
    // ’Tõ5‰ñ‚²‚Æ‚É“G‚ªLv+1‚³‚ê‚éiŠÉ‚â‚©‚È‹­‰»j
    int level = 1 + explorationCount / 5;
    int extra = level - 1;
    return std::make_unique<Enemy>(
        &data, level,
        data.baseHp + extra * 4,
        data.baseAttack + extra,
        data.baseExp + extra * 3);
}

Companion CompanionFactory::Create(const Enemy& defeated)
{
    const EnemyData& d = defeated.Data();
    return Companion(d.name, 1, d.baseHp, d.baseAttack);
}