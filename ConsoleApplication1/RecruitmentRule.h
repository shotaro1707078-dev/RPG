#pragma once
// RecruitmentRule.h
// 仲間加入判定の計算部分だけを分離したもの。
// roll(乱数)を外から渡せるので、固定値でテストできる。
#pragma once

class RecruitmentRule
{
public:
    // roll(0.0以上1.0未満)が rate 未満なら加入成功
    static bool IsRecruited(double rate, double roll) { return roll < rate; }
};