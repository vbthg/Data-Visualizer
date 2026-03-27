#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace Utils
{
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

        // Trạng thái hiển thị cơ bản
        bool isFocused = true;
        float opacity = 255.0f;
    };
}
}
