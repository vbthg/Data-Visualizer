#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <cmath>
#include <algorithm>

#include "Squircle.h"
#include "Spring.h"
#include "ResourceManager.h"

namespace GUI
{
    class Slider
    {
    public:

        // trackWidth: Chiều dài của thanh trượt (không tính icon)
        Slider(float trackWidth = 220.f);

        // 2. Thêm các hàm quản lý Transform thủ công
        void setPosition(sf::Vector2f pos);
        sf::Vector2f getPosition() const { return position; }

        // --- Cấu hình ---
        void setRange(float minVal, float maxVal, float step);
        void setValue(float val, bool instant = false);
        void setOpacity(float opacity); // Dùng cho Morphing (0 -> 255)

        // Cài đặt icon 2 đầu (Dùng string để hỗ trợ Icon Font như Phosphor)
        void setIcons(const sf::Texture& leftTex, const sf::Texture& rightTex);

        // --- Callback ---
        std::function<void(float)> onValueChanged;

        // --- Core ---
        bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void update(const sf::RenderWindow& window, float dt); // Cần window để tính vị trí chuột thực
        void draw(sf::RenderTarget& target);


        float getValue() const { return currentValue; }

    private:

        // --- Helper ---
        void updateVisuals();
        float snap(float value); // Bắt dính vào các mốc step (VD: 0.25)

        // --- Dữ liệu State ---

        sf::Vector2f position;

        float minValue;
        float maxValue;
        float stepSize;
        float currentValue;
        float lastSentValue = -1.0f;

        bool isDragging;
        bool isHovered;

        float trackWidth;
        const float trackHeight = 20.0f; // Track rất hẹp chuẩn iOS

        float opacityFactor = 1.0f;

        // --- Thành phần đồ họa (Visuals) ---
        GUI::Squircle bgTrack;     // Thanh nền (Màu xám nhạt)
        GUI::Squircle activeTrack; // Thanh đã fill (Màu xanh Apple)

        sf::Sprite leftIcon;
        sf::Sprite rightIcon;

        // --- Hệ thống Animation (Lò xo) ---
        Utils::Physics::Spring thumbSpring; // Lò xo mượt hóa vị trí X của con lăn
    };
}
