#pragma once
// Factory.h
// 【技術2：ファクトリーパターン】敵と仲間の生成処理を一か所にまとめる
#pragma once
#include <memory>
#include "Character.h"
#include "EnemyData.h"

class EnemyFactory
{
public:
    // データテーブルの1行と探索回数から、強化済みの敵を生成する
    static std::unique_ptr<Enemy> Create(const EnemyData& data, int explorationCount);
};

class CompanionFactory
{
public:
    // 倒した敵と同じ種類の仲間を生成する
    static Companion Create(const Enemy& defeated);
};
