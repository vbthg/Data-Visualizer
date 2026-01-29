#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "Smoothing.h"
#include "Spring.h"          // <--- Thêm Spring
#include "RoundedRectangleShape.h" // <--- Thay thế ConvexShape
#include "Theme.h"

namespace GUI
{
    // Cấu trúc chứa màu sắc cho 3 trạng thái
    struct ColorState
    {
        sf::Color normal;
        sf::Color hover;
        sf::Color pressed;
    };

    enum class ButtonPreset
    {
        Primary,    // Xanh Apple
        Secondary,  // Xám (Default)
        Ghost,      // Trong suốt
        Danger,     // Đỏ
        Clean       // Trắng sạch
    };

    class Button
    {
    public:
        // Callback
        std::function<void()> onClick;

        Button(const sf::Font& font, const sf::String& text, sf::Vector2f size = {100.f, 40.f});

        // --- CONFIGURATION ---
        void setPosition(sf::Vector2f pos);
        void setSize(sf::Vector2f size);
        void setCornerRadius(float radius);

        void setText(const sf::String& text);
        void setCharacterSize(unsigned int size);
        void setFont(const sf::Font& font);

        // --- CUSTOMIZATION ---
        void setBackgroundColor(sf::Color normal, sf::Color hover, sf::Color pressed);
        void setTextColor(sf::Color normal, sf::Color hover, sf::Color pressed);
        void setTextColor(sf::Color color); // Set 1 màu cho cả 3
        void setOutline(float thickness, sf::Color color);

        void applyPreset(ButtonPreset preset);

        // --- CORE ---
        void update(sf::RenderWindow& window, float dt);
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);

        // Getters
        sf::Vector2f getSize() const { return size; }
        sf::Vector2f getPosition() const { return position; } // Trả về tâm nút
        bool isHovering() const { return isHovered; }

    private:
        // Layout
        sf::Vector2f position;
        sf::Vector2f size;

        // Components
        // Thay sf::ConvexShape bằng RoundedRectangleShape xịn xò
        GUI::RoundedRectangleShape bgShape;
        sf::Text content;

        // Colors
        ColorState bgColors;
        ColorState textColors;
        sf::Color currentBgColor;
        sf::Color currentTextColor;

        // States
        bool isHovered;
        bool isPressed;

        // Physics (Apple Spring)
        Utils::Physics::Spring scaleSpring; // <--- Thay cho currentScale/targetScale cũ

        // Helpers
        void centerText();
    };
}
