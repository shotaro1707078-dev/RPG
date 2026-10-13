// SelfTest.cpp
#include "SelfTest.h"
#include <iostream>
#include <random>
#include <string>
#include "BattleLogEntry.h"
#include "Character.h"
#include "EnemyData.h"
#include "Factory.h"
#include "GameManager.h"
#include "ObjectPool.h"
#include "RecruitmentRule.h"

static int g_fail = 0;

static void Check(const std::string& name, bool ok)
{
    std::cout << "[" << (ok ? "PASS" : "FAIL") << "] " << name << "\n";
    if (!ok) g_fail++;
}

int RunSelfTests()
{
    g_fail = 0;

    // シングルトン
    Check("シングルトン: Instanceは常に同一", &GameManager::Instance() == &GameManager::Instance());

    // 加入判定（固定のrollで境界を確認）
    Check("加入判定: rate0.4, roll0.39 → 成功", RecruitmentRule::IsRecruited(0.4, 0.39));
    Check("加入判定: rate0.4, roll0.40 → 失敗", !RecruitmentRule::IsRecruited(0.4, 0.40));
    Check("加入判定: rate0.0 は常に失敗", !RecruitmentRule::IsRecruited(0.0, 0.0));
    Check("加入判定: rate1.0 は常に成功", RecruitmentRule::IsRecruited(1.0, 0.999));

    // 統計：1万回試行で期待値付近か（1回の結果では判断しない）
    std::mt19937 rng(1);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    int hit = 0, trials = 10000;
    for (int i = 0; i < trials; ++i)
        if (RecruitmentRule::IsRecruited(0.4, dist(rng))) hit++;
    double ratio = static_cast<double>(hit) / trials;
    Check("加入判定: 1万回の成功率が 40%±3% 以内", ratio > 0.37 && ratio < 0.43);

    // ファクトリー・データテーブル
    const EnemyData& slime = EnemyDatabase::Table()[0];
    auto e0 = EnemyFactory::Create(slime, 0);
    Check("敵ファクトリー: 探索0回でLv1・HP20", e0->Level() == 1 && e0->Hp() == 20 && e0->IsAlive());
    auto e10 = EnemyFactory::Create(slime, 10);
    Check("敵ファクトリー: 探索10回でLv3・HP28", e10->Level() == 3 && e10->Hp() == 28);
    Companion comp = CompanionFactory::Create(*e0);
    Check("仲間ファクトリー: 敵と同じ種類の仲間", comp.Name() == "スライム" && comp.Hp() == comp.MaxHp());

    // パーティー上限
    GameManager& gm = GameManager::Instance();
    gm.StartNewGame();
    bool a = gm.TryAddCompanion(comp);
    bool b = gm.TryAddCompanion(comp);
    bool c = gm.TryAddCompanion(comp);
    bool d = gm.TryAddCompanion(comp);
    Check("パーティー上限: 3体まで追加でき4体目は拒否", a && b && c && !d && gm.Party().size() == 3);

    // 成長
    Player p("テスト");
    int ups = p.GainExp(20);
    Check("レベルアップ: EXP20でLv2・最大HP60・攻撃10",
        ups == 1 && p.Level() == 2 && p.MaxHp() == 60 && p.Attack() == 10);

    // 倒れたキャラは行動不能
    Companion dead("テスト仲間", 1, 10, 3);
    dead.TakeDamage(999);
    Check("HP0のキャラは IsAlive が false", !dead.IsAlive() && dead.Hp() == 0);

    // オブジェクトプール
    ObjectPool<BattleLogEntry> pool([](BattleLogEntry& e) { e.Reset(); });
    BattleLogEntry* first = pool.Get();
    first->message = "古いメッセージ";
    pool.Release(first);
    BattleLogEntry* second = pool.Get();
    Check("プール: 返却したオブジェクトが再利用される", first == second);
    Check("プール: 返却時にメッセージがリセットされる", second->message.empty());
    Check("プール: 生成1回・再利用1回", pool.CreatedCount() == 1 && pool.ReuseCount() == 1);

    std::cout << (g_fail == 0 ? "== すべてのテストに成功 ==" : "== 失敗あり ==") << "\n";
    return g_fail == 0 ? 0 : 1;
}