#pragma once
#include <SFML/Graphics.hpp>
#include "DockItem.h"

namespace GUI
{
    class Separator : public DockItem
    {
    private:
        sf::RectangleShape m_line;
        sf::Vector2f m_position;
        float m_width;

    public:
        // Chiều cao truyền vào thường là chiều cao của Dock
        Separator(float dockHeight);
        virtual ~Separator() = default;

        void update(sf::RenderWindow& window, float dt) override;
        void draw(sf::RenderWindow& window) override;
        void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;

        void setPosition(const sf::Vector2f pos) override;
        sf::Vector2f getPosition() const override;
        float getWidth() const override;
    };
}
