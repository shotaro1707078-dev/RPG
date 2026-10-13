#pragma once
// ObjectPool.h
// 【技術6：オブジェクトプール】汎用のオブジェクトプール（テンプレートなのでヘッダのみ）
#pragma once
#include <functional>
#include <memory>
#include <vector>

template <typename T>
class ObjectPool
{
public:
    explicit ObjectPool(std::function<void(T&)> resetFunc) : reset_(resetFunc) {}

    // 取得：余りがあれば再利用、なければ新規生成
    T* Get()
    {
        if (!available_.empty())
        {
            T* p = available_.back();
            available_.pop_back();
            ++reuseCount_;
            return p;
        }
        all_.push_back(std::make_unique<T>());
        ++createdCount_;
        return all_.back().get();
    }

    // 返却：状態をリセットしてプールへ戻す
    void Release(T* item)
    {
        reset_(*item);
        available_.push_back(item);
    }

    int CreatedCount() const { return createdCount_; }
    int ReuseCount() const { return reuseCount_; }
    int AvailableCount() const { return static_cast<int>(available_.size()); }

private:
    std::vector<std::unique_ptr<T>> all_; // 全オブジェクトの所有者
    std::vector<T*> available_;           // 未使用のもの
    std::function<void(T&)> reset_;
    int createdCount_ = 0;
    int reuseCount_ = 0;
};