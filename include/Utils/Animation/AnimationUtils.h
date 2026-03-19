// Tạo file Utils/AnimationUtils.h
#pragma once
#include "AnimationManager.h"
#include "../GUI/NodeUI.h"
#include <cmath>
#include <functional>
#include <algorithm>

namespace Utils
{
    class AnimationUtils
    {
    public:
        static AnimationStep createOrbitalSwap(GUI::NodeUI* node1, GUI::NodeUI* node2, std::function<void()> customStart, std::function<void()> customFinish, float duration = 1.2f, std::string logMsg = "")
        {
            AnimationStep step;
            step.logMessage = logMsg;
            step.duration = duration;

            sf::Vector2f start1 = node1->getCurrentPosition();
            sf::Vector2f start2 = node2->getCurrentPosition();

            step.onStart = [node1, node2, customStart]()
            {
                node1->setTargetColor(sf::Color::Red, sf::Color::White, sf::Color::Red);
                node2->setTargetColor(sf::Color::Red, sf::Color::White, sf::Color::Red);
                if (customStart)
                {
                    customStart();
                }
            };

            step.onUpdate = [node1, node2, start1, start2](float t)
            {
                const float PI = 3.14159265f;
                float easedT = (1.0f - std::cos(t * PI)) / 2.0f;

                sf::Vector2f dir = start2 - start1;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

                sf::Vector2f normal(0.0f, 0.0f);
                if (len > 0.0001f)
                {
                    normal = sf::Vector2f(-dir.y / len, dir.x / len);
                }

                float arcHeight = std::min(len * 0.3f, 150.0f);
                float curve = std::sin(easedT * PI) * 1.2f;

                sf::Vector2f current1 = start1 + (start2 - start1) * easedT + normal * (curve * arcHeight);
                sf::Vector2f current2 = start2 + (start1 - start2) * easedT + normal * (-curve * arcHeight);

                node1->setExactPosition(current1.x, current1.y);
                node2->setExactPosition(current2.x, current2.y);
            };

            step.onFinish = [node1, node2, customFinish]()
            {
                node1->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
                node2->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
                if (customFinish)
                {
                    customFinish();
                }
            };

            return step;
        }
    };
}
