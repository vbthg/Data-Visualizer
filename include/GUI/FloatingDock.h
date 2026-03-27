#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include "Command.h"
#include "DockItem.h"
#include "Button.h"
#include "Separator.h"
#include "Squircle.h"
#include "Spring.h"

namespace GUI
{
    class FloatingDock
    {
    private:
        std::vector<DockItem*> m_items;

        float dockHeight;
        sf::Vector2f position;

        GUI::Squircle background;
        sf::Sprite shadowSprite;

//        const sf::Texture* m_blurTexture = nullptr; // Thêm dòng này

        // Highlighter
        GUI::Squircle highlighter;
        Utils::Physics::Spring highlightX;
        Utils::Physics::Spring highlightWidth;
        float currentHighlightAlpha;
        float targetHighlightAlpha;

        // --- Hệ thống Vật lý (Apple Style) ---
        Utils::Physics::Spring widthSpring; // Quản lý chiều rộng toàn Dock
        Utils::Physics::Spring ySpring;     // Quản lý trượt lên/xuống

        // Hàm nội bộ để dàn đều các component
        void repositionChildren(float currentWidth);

    public:
        std::function<void(sf::Vector2f, DS::Command)> onCommandClicked;

        FloatingDock();
        ~FloatingDock();

        // API MỚI: Thêm Component bất kỳ vào Dock
        void clearItems();
        void addItem(DockItem* item);

        // Hàm tương thích ngược với cấu trúc cũ
        void setCommands(const std::vector<DS::Command>& cmds, const sf::Font& iconFont);
        void setBlurTexture(const sf::Texture& texture, sf::Vector2f size); // Thêm dòng này

        void updateLayout();
        void setPosition(float x, float y);
        sf::Vector2f getPosition() const;
        void setTargetY(float targetY);
        void snapToY(float startY);

        void handleEvent(const sf::Event& event, sf::RenderWindow& window);
        void update(float dt, sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);
    };
}
