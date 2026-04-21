#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "Smoothing.h"
#include "Spring.h"          // <--- Thêm Spring
#include "RoundedRectangleShape.h" // <--- Thay thế ConvexShape
#include "Theme.h"
#include "Squircle.h"
#include "DockItem.h"

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

    class Button : public DockItem
    {
    public:
        // Callback
        std::function<void()> onClick;

        Button(const sf::Font& font, const sf::String& text, sf::Vector2f size = {100.f, 40.f});

        // --- CONFIGURATION ---
        // Sửa lại tham số thành const sf::Vector2f& cho khớp với DockItem
        void setPosition(const sf::Vector2f pos) override;
        void setSize(sf::Vector2f size);
        void setCornerRadius(float radius);
        // Thêm hàm để điều khiển lò xo scale từ bên ngoài
        void setScaleTarget(float targetScale);
        void setMaxScale(float maxScale);

        void setText(const sf::String& text);
        void setCharacterSize(unsigned int size);
        void setFont(const sf::Font& font);
        void triggerTextPop(); // Hàm mới do bạn đề xuất

        // --- CUSTOMIZATION ---
        void setBackgroundColor(sf::Color normal, sf::Color hover, sf::Color pressed);
        void setTextColor(sf::Color normal, sf::Color hover, sf::Color pressed);
        void setTextColor(sf::Color color); // Set 1 màu cho cả 3
        void setOutline(float thickness, sf::Color color);
        void setOpacity(float opacity);
        void setPower(int power);

        void applyPreset(ButtonPreset preset);

        // --- CORE ---
        // Bỏ chữ 'const' ở sf::RenderWindow để khớp với DockItem
        void update(sf::RenderWindow& window, float dt) override;
        void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
        void draw(sf::RenderTarget& target) override;

        // Getters
        sf::Vector2f getSize() const { return size; }
        sf::Vector2f getPosition() const override { return position; } // Trả về tâm nút
        float getScale() const { return scaleSpring.position; }
        float getWidth() const override { return size.x; }

        bool isHovering() const { return isHovered; }

    private:
        // Layout
        sf::Vector2f position;
        sf::Vector2f size;

        // Components
        // Thay sf::ConvexShape bằng RoundedRectangleShape xịn xò
        GUI::Squircle bgShape;
        sf::Text content;

        // Colors
        ColorState bgColors;
        ColorState textColors;
        sf::Color currentBgColor;
        sf::Color currentTextColor;

        float opacityFactor = 1.0f; // 0.0f (Ẩn) -> 1.0f (Hiện nguyên bản)
        float m_baseRadius = 15.0f;
        float m_maxScale = 1.2f;

        // States
        bool isHovered;
        bool isPressed;

        // Physics (Apple Spring)
        Utils::Physics::Spring scaleSpring; // <--- Thay cho currentScale/targetScale cũ
        // Thêm lò xo riêng cho Text
        Utils::Physics::Spring textScaleSpring;

        // Helpers
        void centerText();
    };
}
