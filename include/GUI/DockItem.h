#pragma once
#include <SFML/Graphics.hpp>

namespace GUI
{
    class DockItem
    {
    public:
        virtual ~DockItem() = default;

        // Bắt buộc phải có để Dock gọi vòng lặp
        virtual void update(sf::RenderWindow& window, float dt) = 0;
        virtual void draw(sf::RenderWindow& window) = 0;
        virtual void handleEvent(const sf::Event& event, sf::RenderWindow& window) = 0;

        // Bắt buộc phải có để Dock sắp xếp vị trí
        virtual void setPosition(const sf::Vector2f pos) = 0;
        virtual sf::Vector2f getPosition() const = 0;

        // Bắt buộc phải có để Dock tính toán Target Width (phục vụ co dãn)
        virtual float getWidth() const = 0;

        // Thêm vào public của DockItem.h để Dock biết component nào đang được trỏ chuột
        virtual bool isHovering() const { return false; }
    };
}
