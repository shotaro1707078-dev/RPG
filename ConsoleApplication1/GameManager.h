#pragma once
// GameManager.h
// 【技術4：シングルトン】ゲーム全体の状態を一元管理する
// 【技術5：有限状態機械】Run() が State に応じて処理を切り替える
#pragma once
#include <memory>
#include <random>
#include <string>
#include <vector>
#include "BattleLogEntry.h"
#include "Character.h"
#include "GameState.h"
#include "ObjectPool.h"

class GameManager
{
public:
    static constexpr int MaxCompanions = 3; // 仲間の上限

    // 唯一のインスタンスを返す（初回呼び出し時に生成される）
    static GameManager& Instance();

    // コピー禁止：複数のGameManagerができないようにする
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    GameState State() const { return state_; }
    const Player& GetPlayer() const { return player_; }
    const std::vector<Companion>& Party() const { return party_; }
    int ExplorationCount() const { return explorationCount_; }
    ObjectPool<BattleLogEntry>& LogPool() { return logPool_; }
    void SetSeed(unsigned seed) { rng_.seed(seed); }

    void StartNewGame();                            // 新しい探索を開始（初回・リトライ共通）
    bool TryAddCompanion(const Companion& c);       // 上限なら false
    void Run();                                     // 状態機械のメインループ

private:
    GameManager(); // private：外部から new できない

    // --- 状態ごとの処理 ---
    void HandleExploration();
    void HandleBattle();
    void HandleVictory();
    void HandleRecruitment();
    void HandleGameOver();

    // --- 補助 ---
    void Log(const std::string& message); // プールから取得してログに追加
    void FlushLog();                      // 表示してプールへ返却
    void CleanupParty();                  // 倒れた仲間を離脱させる
    int CalcDamage(const Character& c);
    double NextDouble();
    void PrintStatus() const;
    int ChooseMenu(const std::string& prompt, const std::vector<std::string>& options);

    static constexpr double EncounterRate = 0.7; // 敵との遭遇確率

    GameState state_ = GameState::Exploration;
    Player player_;
    std::vector<Companion> party_;
    int explorationCount_ = 0;
    int totalExp_ = 0;
    std::unique_ptr<Enemy> currentEnemy_;
    std::mt19937 rng_;
    ObjectPool<BattleLogEntry> logPool_;
    std::vector<BattleLogEntry*> pendingLogs_;
};
