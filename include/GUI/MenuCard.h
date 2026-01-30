#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "Button.h"
#include "Squircle.h"
#include "Utils/Graphics/Theme.h"

namespace GUI
{
    enum class CardPos { First, Middle, Last, Single };

    class MenuCard
    {
    public:
        MenuCard(int id, const std::string& title, const std::string& number, sf::Color themeColor);
        ~MenuCard();

        // --- SETTERS: Gán hành động (Callbacks) ---
        void setOnSelect(std::function<void()> callback);
        void setOnViewMore(std::function<void()> callback);
        void setOnStart(std::function<void()> callback);
        void setOnBack(std::function<void()> callback);

        // Layout & State
        void setCardPosition(CardPos pos, float radius);
        void setTarget(const sf::Vector2f& pos, const sf::Vector2f& size);
        void setSelected(bool sel);
        void setExpanded(bool exp);

        // --- HANDLE EVENT: Giờ chỉ cần Event và Window ---
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

        void update(float dt, sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);

        // Getters
        sf::FloatRect getGlobalBounds() const;
        int getId() const { return id; }
        sf::Color getThemeColor() const { return themeColor; }

    private:
        int id;
        sf::Color themeColor;
        bool selected;
        bool expanded;

        // Callbacks (Lưu trữ hàm)
        std::function<void()> actionSelect;
        std::function<void()> actionViewMore;
        std::function<void()> actionStart;
        std::function<void()> actionBack;

        // Animation Vars
        float animTimer;
        sf::Vector2f startPos, currentPos, targetPos;
        sf::Vector2f startSize, currentSize, targetSize;

        // Visuals
        float contentAlpha;
        float cornerRadius;
        CardPos cardPos;

        Squircle bgShape;

        // Content
        sf::Text textNumber;
        sf::Text textTitle;
        sf::Text textBigTitle;
        sf::RectangleShape imgPlaceholder;

        sf::Vector2f currentBigTitlePos;
        sf::Vector2f currentImgPos;

        // Buttons
        Button* btnViewMore;
        Button* btnBack;
        Button* btnStart;
    };
}
