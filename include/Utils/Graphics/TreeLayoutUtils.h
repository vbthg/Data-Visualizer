#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

namespace Utils
{
    class TreeLayoutUtils
    {
    public:
        // Ngăn chặn hoàn toàn việc khởi tạo đối tượng (Strict OOP)
        TreeLayoutUtils() = delete;

        // Trả về tọa độ dựa trên độ sâu và vị trí tương đối (-1.0 là cực trái, 1.0 là cực phải)
        static sf::Vector2f getBinaryTreeNodePos(int depth, float xFactor, sf::Vector2f origin, float hGap, float vGap)
        {
            // Khoảng cách ngang giảm dần theo lũy thừa của 2 để tránh chồng chéo
            float currentHGap = hGap / std::pow(2.0f, depth);

            float x = origin.x + xFactor * currentHGap;
            float y = origin.y + depth * vGap;

            return {x, y};
        }
    };
}
