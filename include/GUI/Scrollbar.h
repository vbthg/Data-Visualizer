#pragma once
#include <SFML/Graphics.hpp>
#include "Squircle.h"
#include "Spring.h"

namespace GUI
{
    class Scrollbar : public sf::Drawable
    {
    private:
        GUI::Squircle m_thumb;

        float m_alpha = 0.f;
        float m_hideTimer = 0.f;
        bool m_isHovered = false;

        // Lò xo điều khiển độ dày (4px -> 8px)
        Utils::Physics::Spring m_thicknessSpring;

        const float m_minThumbHeight = 30.f;
        const float m_maxAlpha = 180.f; // Không bao giờ quá đậm để giữ chất Apple

        bool m_isDragging = false;
        float m_dragOffsetY = 0.f;        // Lưu vị trí chuột so với đỉnh Thumb
        float m_currentThumbHeight = 0.f; // Lưu chiều cao hiện tại để trả về cho Board

    public:
        Scrollbar();

        void show(); // Gọi khi có sự kiện cuộn
        void update(float dt, float currentScroll, float contentHeight, float viewportHeight, const sf::Vector2f& localMouse);


        // SETTERS
        void setDragging(bool dragging)
        {
            m_isDragging = dragging;
        }

        void setDragOffset(float offset)
        {
            m_dragOffsetY = offset;
        }

        // Kiểm tra xem chuột có nằm ĐÈ lên viên thuốc (Thumb) không
        bool isMouseOverThumb(const sf::Vector2f& localMouse) const
        {
            return m_thumb.getGlobalBounds().contains(localMouse);
        }

        bool isHovered() const { return m_isHovered; }
        bool isDragging() const { return m_isDragging; }
        // Lấy vị trí Y hiện tại của Thumb để tính Offset
        float getThumbY() const { return m_thumb.getPosition().y; }
        float getDragOffset() const
        {
            return m_dragOffsetY;
        }

        float getThumbHeight() const
        {
            return m_currentThumbHeight;
        }



        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}
