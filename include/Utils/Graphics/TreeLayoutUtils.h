#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "NodeUI.h"

namespace Utils
{
    class TreeLayoutUtils
    {
    public:
        // Áp dụng cho cấu trúc mảng (Heap)
        static void updateHeapLayout(const std::vector<GUI::NodeUI*>& nodes, sf::Vector2f rootPos, float minLeafSpacing = 60.0f, float levelHeight = 80.0f)
        {
            if (nodes.empty())
            {
                return;
            }

            int total = nodes.size();
            int maxDepth = (int)std::log2(total);

            // Gán mục tiêu cho lò xo của Root
            nodes[0]->setTargetPosition(rootPos.x, rootPos.y);

            for (int i = 0; i < total; ++i)
            {
                int left = 2 * i + 1;
                int right = 2 * i + 2;
                int currentDepth = (int)std::log2(i + 1);

                // Tính khoảng cách văng ra hai bên
                float offsetX = minLeafSpacing * std::pow(2.0f, std::max(0, maxDepth - currentDepth - 1));

                // Lấy tọa độ đích (target) của Node cha để cộng dồn
                sf::Vector2f parentPos = nodes[i]->getTargetPosition();

                if (left < total)
                {
                    nodes[left]->setTargetPosition(parentPos.x - offsetX, parentPos.y + levelHeight);
                }

                if (right < total)
                {
                    nodes[right]->setTargetPosition(parentPos.x + offsetX, parentPos.y + levelHeight);
                }
            }
        }
    };
}
