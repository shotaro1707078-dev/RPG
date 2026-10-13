#pragma once
// BattleLogEntry.h
// 【技術6：オブジェクトプール】プールで再利用される戦闘ログ1行ぶん
#pragma once
#include <string>

struct BattleLogEntry
{
    std::string message;
    void Reset() { message.clear(); } // 返却時に古いメッセージを消す
};