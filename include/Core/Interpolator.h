#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include "AnimationMetadata.h"
#include "Easing.h"

namespace Utils
{
namespace Math
{
    class Interpolator
    {
    public:
        // 1. Hàm tính toán Tọa độ (Nhận toàn bộ NodeState để lấy arcPivot)
        static sf::Vector2f calculatePosition(const Core::NodeState& start, const Core::NodeState& end, float alpha, Core::TransitionType type)
        {
            if(alpha <= 0.0f) return start.position;
            if(alpha >= 1.0f) return end.position;

            if (type == Core::TransitionType::Orbital)
            {
                sf::Vector2f dir = end.position - start.position;
                float distSq = dir.x * dir.x + dir.y * dir.y; // Dùng bình phương khoảng cách

                if (distSq < 0.0001f) return start.position;

                float dist = std::sqrt(distSq);
                sf::Vector2f center = (start.position + end.position) / 2.f;
                sf::Vector2f v_perp(-dir.y, dir.x);

                // Center cũng nên mượt
                center += (v_perp / dist) * (dist * 0.3f);

                float startAngle = std::atan2(start.position.y - center.y, start.position.x - center.x);
                float endAngle = std::atan2(end.position.y - center.y, end.position.x - center.x);

                // Chuẩn hóa góc (Wrap angle)
                while (endAngle - startAngle > PI)  endAngle -= 2.f * PI;
                while (endAngle - startAngle < -PI) endAngle += 2.f * PI;

                float currentAngle = Easing::lerp(startAngle, endAngle, alpha);
                float radius = std::sqrt(std::pow(start.position.x - center.x, 2) + std::pow(start.position.y - center.y, 2));

                return { center.x + radius * std::cos(currentAngle), center.y + radius * std::sin(currentAngle) };
            }
            else if(type == Core::TransitionType::ArcSwing)
            {
                // Lấy tâm quay từ trạng thái đích
                sf::Vector2f pivot = end.arcPivot;

                float startAngle = std::atan2(start.position.y - pivot.y, start.position.x - pivot.x);
                float endAngle = std::atan2(end.position.y - pivot.y, end.position.x - pivot.x);

                if(endAngle - startAngle > PI) endAngle -= 2.0f * PI;
                if(endAngle - startAngle < -PI) endAngle += 2.0f * PI;

                float currentAngle = Easing::lerp(startAngle, endAngle, alpha);

                // Nội suy bán kính đề phòng chiều dài cạnh bị thay đổi sau khi xoay cây
                float startRadius = std::sqrt(std::pow(start.position.x - pivot.x, 2) + std::pow(start.position.y - pivot.y, 2));
                float endRadius = std::sqrt(std::pow(end.position.x - pivot.x, 2) + std::pow(end.position.y - pivot.y, 2));
                float currentRadius = Easing::lerp(startRadius, endRadius, alpha);

                return sf::Vector2f(
                    pivot.x + currentRadius * std::cos(currentAngle),
                    pivot.y + currentRadius * std::sin(currentAngle)
                );
            }
            else if(type == Core::TransitionType::FadeOnly)
            {
                return start.position;
            }

            return Easing::lerp(start.position, end.position, alpha);
        }

        // 2. Hàm nội suy toàn bộ trạng thái Node
        static Core::NodeState interpolateNode(const Core::NodeState& start, const Core::NodeState& end, float alpha, Core::TransitionType type)
        {
            Core::NodeState result;
            result.id = start.id;
            result.value = end.value;
            result.subText = end.subText;

            // Cập nhật lại lời gọi hàm calculatePosition
            result.position = calculatePosition(start, end, alpha, type);
            result.arcPivot = end.arcPivot;

            result.scale = Easing::lerp(start.scale, end.scale, alpha);
            result.opacity = Easing::lerp(start.opacity, end.opacity, alpha);

//            result.fillColor = Easing::lerpColor(start.fillColor, end.fillColor, alpha);
//            result.outlineColor = Easing::lerpColor(start.outlineColor, end.outlineColor, alpha);
//            result.textColor = Easing::lerpColor(start.textColor, end.textColor, alpha);

            if(alpha > 0.01f)
            {
                result.fillColor = end.fillColor;
                result.outlineColor = end.outlineColor;
                result.textColor = end.textColor;
                result.isDraggable = end.isDraggable;
            }
            else
            {
                result.fillColor = start.fillColor;
                result.outlineColor = start.outlineColor;
                result.textColor = start.textColor;
                result.isDraggable = start.isDraggable;
            }

            return result;
        }

        // 3. Hàm nội suy toàn bộ trạng thái Edge
        static Core::EdgeState interpolateEdge(const Core::EdgeState& start, const Core::EdgeState& end, float alpha)
        {
            Core::EdgeState result;
            result.startNodeId = start.startNodeId;
            result.endNodeId = start.endNodeId;

            // 1. Nội suy tiến trình đổ màu (Dòng chảy phải mượt)
//            result.fillProgress = Easing::lerp(start.fillProgress, end.fillProgress, alpha);
            result.fillProgress = start.fillProgress + Utils::Math::Easing::easeOutCubic(alpha) * (end.fillProgress - start.fillProgress);

            // 2. Nội suy độ mờ (Cần mượt để hiện lên/biến mất sang trọng)
            result.opacity = Easing::lerp(start.opacity, end.opacity, alpha);

            // 3. Giữ nguyên các cờ trạng thái
            result.fillFromStart = end.fillFromStart;
            result.isPulsing = end.isPulsing;
            result.isFocused = (alpha < 0.5f) ? start.isFocused : end.isFocused;

            // 4. COLOR SNAP LOGIC: Không dùng lerp cho màu sắc trạng thái
            // Nếu bắt đầu chuyển sang frame mới, dùng luôn màu của frame đích
            // để cái "dòng chảy" fillProgress mang đúng màu sắc của trạng thái tiếp theo.
            if(alpha > 0.01f)
            {
                result.baseFillColor = end.baseFillColor;
                result.fillColor = end.fillColor;
                result.pulseColor = end.pulseColor;
            }
            else
            {
                result.baseFillColor = start.baseFillColor;
                result.fillColor = start.fillColor;
                result.pulseColor = start.pulseColor;
            }

            return result;
        }
    };
}
}
