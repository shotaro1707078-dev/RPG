// main.cpp
// アプリケーションの起動だけを担当する
//   ./game            通常プレイ
//   ./game --test     自己テスト
//   ./game --seed 123 乱数固定
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "GameManager.h"
#include "SelfTest.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[])
{


    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--test") == 0)
            return RunSelfTests();
        if (std::strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
            GameManager::Instance().SetSeed(static_cast<unsigned>(std::atoi(argv[i + 1])));
    }

    std::cout << "★★★ 無限の道と仲間たち ★★★\n";
    GameManager::Instance().Run();
    return 0;
}