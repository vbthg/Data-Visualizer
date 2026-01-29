#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

#include "Button.h"
#include "Squircle.h"
#include "Easing.h" // <--- 1. Dùng Spring
#include "Theme.h"  // <--- 2. Dùng Theme

namespace GUI
{
    enum class CardPos { First, Middle, Last, Single };

    class MenuCard
    {
    public:
        MenuCard(int id, const std::string& title, const std::string& number, sf::Color themeColor);
        ~MenuCard();

        void setCardPosition(CardPos pos, float radius);

        // Thay vì reset timer, hàm này giờ chỉ set target cho Spring
        void setTarget(const sf::Vector2f& pos, const sf::Vector2f& size);

        void setSelected(bool selected);
        void setExpanded(bool expanded);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window,
                         std::function<void()> onSelect,
                         std::function<void()> onViewMore,
                         std::function<void()> onStart,
                         std::function<void()> onBack);

        void update(float dt, sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);

        int getId() const { return id; }
        sf::Color getThemeColor() const { return themeColor; }


    private:
        int id;
        sf::Color themeColor;
        bool selected;
        bool expanded;

        // --- ANIMATION STATE (EASING) ---
        float animTimer; // Thời gian đã chạy

        // Cần lưu 3 trạng thái để Lerp: Bắt đầu -> Hiện tại -> Đích
        sf::Vector2f startPos, currentPos, targetPos;
        sf::Vector2f startSize, currentSize, targetSize;

        // Props layout
        CardPos cardPos;
        float cornerRadius;

        // --- VISUALS ---
        float contentAlpha;
        Squircle bgShape; // Dùng Squircle "xịn"

        // --- CONTENT ---
        sf::Text textNumber;
        sf::Text textTitle;
        sf::Text textBigTitle;
        sf::RectangleShape imgPlaceholder;

        // --- SWIPE STATE ---
        sf::Vector2f currentBigTitlePos;
        sf::Vector2f currentImgPos;

        // --- BUTTONS ---
        Button* btnViewMore;
        Button* btnBack;
        Button* btnStart;
    };
}
