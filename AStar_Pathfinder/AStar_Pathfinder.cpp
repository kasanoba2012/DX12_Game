// AStar_Pathfinder.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "AStar.h"

int main()
{
	AStar::MapGenerator gen;
	// 전체 맵 크기 세팅
	gen.SetWorldSize({ 8,5 });
	// 벽 세팅        x , y
    gen.AddColision({ 1, 1 });
    gen.AddColision({ 1, 2 });
    gen.AddColision({ 1, 3 });
    gen.AddColision({ 4, 0 });
    gen.AddColision({ 4, 1 });
    gen.AddColision({ 4, 2 });
    gen.AddColision({ 4, 3 });
    gen.AddColision({ 2, 3 });
    gen.AddColision({ 3, 3 });

    // 길 찾기 (시작좌표, 목적좌표)
    auto path = gen.findpath({ 2,2 }, { 6, 2 });

    std::reverse(path.begin(), path.end());

    for (auto& index : path)
    {
        std::cout << index.x << " " << index.y << "\n";
    }
    std::cout << "Ending!\n";
}