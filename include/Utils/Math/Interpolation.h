#pragma once
#include <SFML/Graphics.hpp>
#include "AnimationMetadata.h"
#include "Easing.h"

namespace Utils
{
namespace Math
{
    class Interpolator
    {
    public:
        static sf::Vector2f calculate(sf::Vector2f start, sf::Vector2f end, float alpha, Core::TransitionType type)
        {
            if(alpha <= 0.0f) return start;
            if(alpha >= 1.0f) return end;

            switch(type)
            {
                case Core::TransitionType::Orbital:
                {
                    // Tâm quay là trung điểm của Start và End
                    sf::Vector2f center = (start + end) / 2.0f;

                    // Thêm một chút Offset lên trên để tạo cung tròn (Arc)
                    // v_perp là vector vuông góc với đường nối 2 node
                    sf::Vector2f dir = end - start;
                    sf::Vector2f v_perp(-dir.y, dir.x);
                    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

                    // Độ cong của cung (hàng số 0.5f có thể điều chỉnh)
                    center += (v_perp / dist) * (dist * 0.3f);

                    float radius = std::sqrt(std::pow(start.x - center.x, 2) + std::pow(start.y - center.y, 2));
                    float startAngle = std::atan2(start.y - center.y, start.x - center.x);
                    float endAngle = std::atan2(end.y - center.y, end.x - center.x);

                    // Xử lý góc để luôn quay theo cung ngắn nhất hoặc chiều thuận
                    if(endAngle - startAngle > PI) endAngle -= 2.0f * PI;
                    if(endAngle - startAngle < -PI) endAngle += 2.0f * PI;

                    float currentAngle = startAngle + (endAngle - startAngle) * alpha;

                    return sf::Vector2f(
                        center.x + radius * std::cos(currentAngle),
                        center.y + radius * std::sin(currentAngle)
                    );
                }

                case Core::TransitionType::ArcSwing:
                {
                    // Dành cho Rotation (Xoay cây) - Thường xoay quanh Node cha
                    // Sẽ thảo luận sâu hơn khi làm AVL, hiện tại dùng Linear tạm
                    return start + (end - start) * alpha;
                }

                case Core::TransitionType::Linear:
                default:
                    return start + (end - start) * alpha;
            }
        }
    };
}
}
