#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace Core
{
    enum class TransitionType
    {
        Linear,
        Orbital,
        ArcSwing,
        FadeOnly
    };

    struct NodeState
    {
        int id;
        sf::Vector2f position;

        // MỚI THÊM: Điểm tựa để xoay (Dùng riêng cho ArcSwing)
        sf::Vector2f arcPivot;

        bool isDraggable = false;

        // THÊM DÒNG NÀY: Để mỗi Node biết mình nên bay kiểu gì (Linear, Orbital, ...)
        TransitionType transition = TransitionType::Linear;

        sf::Color fillColor = sf::Color::White;
        sf::Color outlineColor = sf::Color(200, 200, 200);
        sf::Color textColor = sf::Color::Black;

        float scale = 1.0f;
        float opacity = 255.0f;

        std::string value;
        std::string subText; // Hiển thị Balance Factor, Index, v.v.
    };

    struct EdgeState
    {
        int startNodeId;
        int endNodeId;

        // Thông số nội suy cho hiệu ứng lan màu và hạt bay
        float fillProgress = 0.0f;
        bool fillFromStart = true;
        sf::Color baseFillColor = sf::Color(150, 150, 150, 0);
        sf::Color fillColor = sf::Color::White;

        bool isPulsing = false;
        float pulseProgress = 0.0f;
        sf::Color pulseColor = sf::Color::Yellow;

        sf::Color targetColor = sf::Color(150, 150, 150);

        // Trạng thái hiển thị cơ bản
        bool isFocused = true;
        float opacity = 1.0f;
    };

    // THÊM DÒNG NÀY VÀO CUỐI NAMESPACE Core
    struct RenderFrame
    {
        std::vector<NodeState> nodes;
        std::vector<EdgeState> edges;
    };
}

