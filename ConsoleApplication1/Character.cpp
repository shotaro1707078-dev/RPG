#include "Character.h"
// Character.cpp
#include "Character.h"
#include <algorithm>

// ---------- Character ----------
Character::Character(const std::string& name, int level, int maxHp, int attack)
    : name_(name), level_(level), maxHp_(maxHp), hp_(maxHp), attack_(attack) {
}

int Character::TakeDamage(int damage)
{
    if (!IsAlive()) return 0;
    hp_ = std::max(0, hp_ - damage);
    return damage;
}

void Character::HealPercent(int percent)
{
    if (!IsAlive()) return;
    hp_ = std::min(maxHp_, hp_ + std::max(1, maxHp_ * percent / 100));
}

// ---------- Player ----------
Player::Player(const std::string& name)
    : Character(name, 1, 50, 8), exp_(0) {
}

int Player::GainExp(int amount)
{
    exp_ += amount;
    int ups = 0;
    while (exp_ >= ExpToNext())
    {
        exp_ -= ExpToNext();
        level_++;
        maxHp_ += 10;  // 最大HP上昇
        attack_ += 2;  // 攻撃力上昇
        hp_ = maxHp_;  // レベルアップ時にHP全回復
        ups++;
    }
    return ups;
}

// ---------- Enemy ----------
Enemy::Enemy(const EnemyData* data, int level, int maxHp, int attack, int expReward)
    : Character(data->name, level, maxHp, attack), data_(data), expReward_(expReward) {
}

// ---------- Companion ----------
Companion::Companion(const std::string& name, int level, int maxHp, int attack)
    : Character(name, level, maxHp, attack) {
}