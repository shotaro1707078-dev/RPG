// Character.h

#pragma once
#include <string>
#include "EnemyData.h"

// 全キャラクター共通：生成した瞬間からHP満タンの正しい状態にする
class Character
{
public:
    Character(const std::string& name, int level, int maxHp, int attack);
    virtual ~Character() = default;

    const std::string& Name() const { return name_; }
    int Level() const { return level_; }
    int MaxHp() const { return maxHp_; }
    int Hp() const { return hp_; }
    int Attack() const { return attack_; }
    bool IsAlive() const { return hp_ > 0; } // HP0以下は行動不可（ルール4）
    std::string Label() const { return name_ + " Lv" + std::to_string(level_); }

    int TakeDamage(int damage);     // 被ダメージ（倒れていれば何も起きない）
    void HealPercent(int percent);  // 最大HPの percent% 回復（最低1）

protected:
    std::string name_;
    int level_;
    int maxHp_;
    int hp_;
    int attack_;
};

// プレイヤー：経験値とレベルアップ
class Player : public Character
{
public:
    explicit Player(const std::string& name);

    int Exp() const { return exp_; }
    int ExpToNext() const { return 10 + level_ * 10; } // 必要経験値の計算式
    int GainExp(int amount); // 経験値を加え、上がったレベル数を返す

private:
    int exp_;
};

// 敵：種類データへのポインタと獲得経験値を持つ
class Enemy : public Character
{
public:
    Enemy(const EnemyData* data, int level, int maxHp, int attack, int expReward);

    const EnemyData& Data() const { return *data_; }
    int ExpReward() const { return expReward_; }

private:
    const EnemyData* data_;
    int expReward_;
};

// 仲間
class Companion : public Character
{
public:
    Companion(const std::string& name, int level, int maxHp, int attack);
};