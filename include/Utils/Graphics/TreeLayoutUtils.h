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
        // Trả về danh sách tọa độ cho toàn bộ Heap
        static std::vector<sf::Vector2f> calculateHeapPositions(int totalNodes, sf::Vector2f rootPos)
        {
            if(totalNodes <= 0) return {};

            std::vector<sf::Vector2f> positions(totalNodes);
            int maxDepth = (int)std::log2(totalNodes);

            float baseLeafSpacing = 70.0f;
            float baseLevelHeight = 100.0f;

            // Hệ số co dãn chiều ngang để không bị quá rộng khi cây sâu
            float compression = (maxDepth > 3) ? std::pow(0.8f, maxDepth - 3) : 1.0f;

            positions[0] = rootPos;

            for(int i = 0; i < totalNodes; ++i)
            {
                int left = 2 * i + 1;
                int right = 2 * i + 2;
                int d = (int)std::log2(i + 1);

                float offsetX = (baseLeafSpacing * compression) * std::pow(2.0f, std::max(0, maxDepth - d - 1));
                float currentLevelHeight = baseLevelHeight * (1.0f + d * 0.05f);

                if(left < totalNodes)
                {
                    positions[left] = sf::Vector2f(positions[i].x - offsetX, positions[i].y + currentLevelHeight);
                }
                if(right < totalNodes)
                {
                    positions[right] = sf::Vector2f(positions[i].x + offsetX, positions[i].y + currentLevelHeight);
                }
            }
            return positions;
        }
    };
}
