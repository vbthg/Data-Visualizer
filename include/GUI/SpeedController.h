#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "DockItem.h"
#include "Button.h"
#include "Slider.h"
#include "Spring.h"

namespace GUI
{
    class SpeedController : public DockItem
    {
    private:
        // --- Components ---
        Button m_mainButton;
        Slider m_slider;

        // --- States ---
        bool m_isExpanded;
        bool m_isPressing;
        float m_pressTimer;

        // Kích thước tĩnh để làm mốc nội suy
        float m_collapsedWidth;
        float m_expandedWidth;

        sf::Vector2f m_position; // Tâm của toàn bộ component

        // Lò xo vật lý quản lý bề ngang
        Utils::Physics::Spring m_widthSpring;

        // --- Logic Data ---
        std::vector<float> m_quickSpeeds;
        int m_currentSpeedIdx;

        void cycleSpeed();
        void collapse();

    public:
        SpeedController(const sf::Font& font);
        virtual ~SpeedController() = default;

        void update(sf::RenderWindow& window, float dt) override;
        void draw(sf::RenderTarget& target) override;
        void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;

        void setPosition(const sf::Vector2f pos) override;
        sf::Vector2f getPosition() const override;
        float getWidth() const override;
    };
}
