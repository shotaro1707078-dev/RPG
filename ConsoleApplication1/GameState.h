#pragma once
// GameState.h
// 【技術5：有限状態機械】ゲームの進行状態を定義する
#pragma once

enum class GameState
{
    Exploration, // 探索中
    Battle,      // 戦闘中
    Victory,     // 戦闘勝利後（経験値獲得）
    Recruitment, // 仲間加入判定中
    GameOver,    // ゲームオーバー
    Exit         // ゲーム終了
};