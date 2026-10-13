#include "GameManager.h"
// GameManager.cpp
#include "GameManager.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include "EnemyData.h"
#include "Factory.h"
#include "RecruitmentRule.h"

// ===== シングルトン =====
GameManager& GameManager::Instance()
{
    static GameManager instance;
    return instance;
}

GameManager::GameManager()
    : player_("勇者"), rng_(std::random_device{}()),
    logPool_([](BattleLogEntry& e) { e.Reset(); })
{
    StartNewGame();
}

void GameManager::StartNewGame()
{
    player_ = Player("勇者");
    party_.clear();
    explorationCount_ = 0;
    totalExp_ = 0;
    currentEnemy_.reset();
    pendingLogs_.clear();
    state_ = GameState::Exploration;
}

bool GameManager::TryAddCompanion(const Companion& c)
{
    if (static_cast<int>(party_.size()) >= MaxCompanions) return false; // ルール7
    party_.push_back(c);
    return true;
}

// ===== 有限状態機械のメインループ =====
void GameManager::Run()
{
    while (state_ != GameState::Exit)
    {
        switch (state_)
        {
        case GameState::Exploration: HandleExploration(); break;
        case GameState::Battle:      HandleBattle();      break;
        case GameState::Victory:     HandleVictory();     break;
        case GameState::Recruitment: HandleRecruitment(); break;
        case GameState::GameOver:    HandleGameOver();    break;
        default: break;
        }
    }
    std::cout << "遊んでくれてありがとう！\n";
}

// ===== 状態：探索 =====
void GameManager::HandleExploration()
{
    std::cout << "\n==================== 探索 ====================\n";
    PrintStatus();
    int choice = ChooseMenu("どの道へ進む？", { "左の道", "中央の道", "右の道", "ゲーム終了" });
    if (choice == 3)
    {
        state_ = GameState::Exit; // Exploration → Exit
        return;
    }

    const char* names[] = { "左の道", "中央の道", "右の道" };
    explorationCount_++;
    std::cout << names[choice] << "を進んだ…（探索回数 " << explorationCount_ << "）\n";

    // 歩くたびに全員が少し回復する（無限に探索できるようにする調整）
    player_.HealPercent(10);
    for (auto& c : party_) c.HealPercent(10);

    if (NextDouble() < EncounterRate)
    {
        // ファクトリーで敵を生成
        const EnemyData& data = EnemyDatabase::PickRandom(rng_, explorationCount_);
        currentEnemy_ = EnemyFactory::Create(data, explorationCount_);
        std::cout << "！！ " << currentEnemy_->Label() << " が現れた！\n";
        state_ = GameState::Battle; // Exploration → Battle
    }
    else
    {
        std::cout << "何も起こらなかった。\n";
    }
}

// ===== 状態：戦闘（1回の呼び出し＝1ターン） =====
void GameManager::HandleBattle()
{
    Enemy& enemy = *currentEnemy_;
    std::cout << "\n-------------------- 戦闘 --------------------\n";
    std::cout << "敵：" << enemy.Label() << "  HP " << enemy.Hp() << "/" << enemy.MaxHp() << "\n";
    PrintStatus();

    int cmd = ChooseMenu("行動を選んでください", { "攻撃", "逃げる" });

    // 逃げる：経験値も加入判定もなしで探索に戻る（ルール8）
    if (cmd == 1)
    {
        std::cout << "うまく逃げ切った！\n";
        CleanupParty();
        currentEnemy_.reset();
        state_ = GameState::Exploration; // Battle → Exploration
        return;
    }

    // 1. プレイヤーの攻撃
    int dmg = CalcDamage(player_);
    enemy.TakeDamage(dmg);
    Log(player_.Name() + "の攻撃！ " + enemy.Name() + "に" + std::to_string(dmg) + "のダメージ");

    // 2. 生存している仲間が順番に攻撃
    for (auto& c : party_)
    {
        if (!enemy.IsAlive()) break;
        if (!c.IsAlive()) continue;
        int cd = CalcDamage(c);
        enemy.TakeDamage(cd);
        Log("仲間の" + c.Name() + "の攻撃！ " + enemy.Name() + "に" + std::to_string(cd) + "のダメージ");
    }

    // 敵が倒れたら敵の攻撃はしない（ルール5）
    if (!enemy.IsAlive())
    {
        Log(enemy.Name() + "を倒した！");
        FlushLog();
        CleanupParty();
        state_ = GameState::Victory; // Battle → Victory
        return;
    }

    // 3. 敵の攻撃：生存している味方からランダムに1体
    std::vector<Character*> targets;
    targets.push_back(&player_);
    for (auto& c : party_)
        if (c.IsAlive()) targets.push_back(&c);
    std::uniform_int_distribution<size_t> pick(0, targets.size() - 1);
    Character* target = targets[pick(rng_)];
    int ed = CalcDamage(enemy);
    target->TakeDamage(ed);
    Log(enemy.Name() + "の攻撃！ " + target->Name() + "に" + std::to_string(ed) + "のダメージ");
    if (!target->IsAlive() && target != &player_)
        Log("仲間の" + target->Name() + "は倒れてしまった…");

    FlushLog();

    // 4. 戦闘終了判定
    if (!player_.IsAlive())
        state_ = GameState::GameOver; // Battle → GameOver
    // それ以外は Battle のまま → 次のターン
}

// ===== 状態：勝利 =====
void GameManager::HandleVictory()
{
    int exp = currentEnemy_->ExpReward();
    totalExp_ += exp;
    std::cout << "経験値を " << exp << " 獲得！\n";
    int ups = player_.GainExp(exp);
    if (ups > 0)
    {
        std::cout << "★ レベルアップ！ Lv" << player_.Level() << " になった！（最大HP "
            << player_.MaxHp() << " / 攻撃力 " << player_.Attack() << "、HP全回復）\n";
    }
    state_ = GameState::Recruitment; // Victory → Recruitment
}

// ===== 状態：仲間加入判定 =====
void GameManager::HandleRecruitment()
{
    const EnemyData& data = currentEnemy_->Data();
    if (RecruitmentRule::IsRecruited(data.recruitRate, NextDouble()))
    {
        std::cout << data.name << "が仲間になりたそうにしている！\n";
        Companion companion = CompanionFactory::Create(*currentEnemy_); // ファクトリー
        if (TryAddCompanion(companion))
            std::cout << "★ " << companion.Name() << "が仲間になった！（"
            << party_.size() << "/" << MaxCompanions << "）\n";
        else
            std::cout << "しかしパーティーが満員なので、加入は見送った。\n";
    }
    else
    {
        std::cout << data.name << "は仲間にならなかった。\n";
    }
    currentEnemy_.reset();
    state_ = GameState::Exploration; // Recruitment → Exploration
}

// ===== 状態：ゲームオーバー =====
void GameManager::HandleGameOver()
{
    int alive = static_cast<int>(std::count_if(party_.begin(), party_.end(),
        [](const Companion& c) { return c.IsAlive(); }));
    std::cout << "\n================ GAME OVER ================\n";
    std::cout << "到達した探索回数 : " << explorationCount_ << "\n";
    std::cout << "プレイヤーLv     : " << player_.Level() << "\n";
    std::cout << "仲間の数         : " << alive << "\n";
    std::cout << "獲得した経験値   : " << totalExp_ << "\n";
    std::cout << "[ログプール] 生成 " << logPool_.CreatedCount()
        << "回 / 再利用 " << logPool_.ReuseCount() << "回\n";

    int choice = ChooseMenu("もう一度遊びますか？", { "もう一度遊ぶ", "終了する" });
    if (choice == 0)
        StartNewGame();           // GameOver → Exploration（リトライ）
    else
        state_ = GameState::Exit; // GameOver → Exit
}

// ===== 戦闘ログ（オブジェクトプールの使用箇所） =====
void GameManager::Log(const std::string& message)
{
    BattleLogEntry* entry = logPool_.Get(); // 取得
    entry->message = message;
    pendingLogs_.push_back(entry);
}

void GameManager::FlushLog()
{
    for (BattleLogEntry* entry : pendingLogs_)
    {
        std::cout << "  " << entry->message << "\n"; // 使用
        logPool_.Release(entry);                     // 返却（中身がリセットされる）
    }
    pendingLogs_.clear();
}

// ===== 補助 =====
void GameManager::CleanupParty()
{
    // 倒れた仲間は離脱し、以後の戦闘で行動しない（ルール6）
    party_.erase(std::remove_if(party_.begin(), party_.end(),
        [](const Companion& c) { return !c.IsAlive(); }),
        party_.end());
}

int GameManager::CalcDamage(const Character& c)
{
    std::uniform_int_distribution<int> var(-1, 1); // 攻撃力 ±1
    return std::max(1, c.Attack() + var(rng_));
}

double GameManager::NextDouble()
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng_);
}

void GameManager::PrintStatus() const
{
    std::cout << player_.Label() << "  HP " << player_.Hp() << "/" << player_.MaxHp()
        << "  攻撃 " << player_.Attack()
        << "  EXP " << player_.Exp() << "/" << player_.ExpToNext() << "\n";
    if (party_.empty())
    {
        std::cout << "仲間：なし（0/" << MaxCompanions << "）\n";
        return;
    }
    std::cout << "仲間（" << party_.size() << "/" << MaxCompanions << "）：";
    for (size_t i = 0; i < party_.size(); ++i)
    {
        if (i > 0) std::cout << " / ";
        std::cout << party_[i].Name() << " HP" << party_[i].Hp() << "/" << party_[i].MaxHp();
    }
    std::cout << "\n";
}

// 番号入力によるメニュー選択（戻り値は0始まり）
int GameManager::ChooseMenu(const std::string& prompt, const std::vector<std::string>& options)
{
    std::cout << prompt << "\n";
    for (size_t i = 0; i < options.size(); ++i)
        std::cout << "  " << (i + 1) << ": " << options[i] << "\n";

    while (true)
    {
        std::cout << "番号(1-" << options.size() << ")> " << std::flush;
        std::string line;
        if (!std::getline(std::cin, line))
            return static_cast<int>(options.size()) - 1; // 入力終了時は最後の項目
        std::istringstream iss(line);
        int n = 0;
        if ((iss >> n) && n >= 1 && n <= static_cast<int>(options.size()))
            return n - 1;
        std::cout << "正しい番号を入力してください。\n";
    }
}