// EnemyData.h
// 【技術3：データテーブル】敵の設定値(EnemyData)とその一覧表(EnemyDatabase)
#pragma once
#include <random>
#include <string>
#include <vector>

// 敵1種類ぶんの設定値
// 【技術1：コンストラクタ】全項目を生成時に初期化する
struct EnemyData
{
    std::string name;
    int baseHp;
    int baseAttack;
    int baseExp;
    double recruitRate; // 仲間加入確率 0.0～1.0
    int minExploration; // 出現し始める探索回数

    EnemyData(const std::string& n, int hp, int atk, int exp, double rate, int minExp)
        : name(n), baseHp(hp), baseAttack(atk), baseExp(exp),
        recruitRate(rate), minExploration(minExp) {
    }
};

// データテーブル本体（敵を増やすときは EnemyData.cpp の表に1行足すだけ）
class EnemyDatabase
{
public:
    static const std::vector<EnemyData>& Table();
    // 現在の探索回数で出現できる敵から1種類をランダムに選ぶ
    static const EnemyData& PickRandom(std::mt19937& rng, int explorationCount);
};