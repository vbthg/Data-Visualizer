#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <sstream>

#include "Button.h"
#include "Squircle.h"
#include "Spring.h"
#include "wrapText.h"
#include "Utils/Graphics/Theme.h"

namespace GUI
{
    // [1] Struct cấu hình
    struct CardConfig {
        int id;
        std::string title;
        std::string number;
        std::string description; // Thêm dòng này
        std::string shortDescription; // [MỚI] Dùng cho Un-expanded

        sf::Texture* iconTexture; // Quan trọng: Dùng Texture pointer
        sf::Color themeColor;

        sf::Color colorTitleUnselected; // [MỚI] Màu Title khi chưa chọn
        sf::Color colorTitleSelected;   // [MỚI] Màu Title khi được chọn
        sf::Color colorNumber; // Thêm dòng này

        // [MỚI] Kích thước khởi tạo (MenuState sẽ truyền vào)
        sf::Vector2f initialSize;
    };

    enum class CardPos { First, Middle, Last, Single };

    class MenuCard
    {
    public:
        MenuCard(const CardConfig& config); // Constructor nhận Config
        ~MenuCard();

        void update(float dt, sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);
        void handleEvent(const sf::Event& event, sf::RenderWindow& window);

        // --- Logic Di chuyển & Trạng thái ---
        void setTarget(const sf::Vector2f& pos, const sf::Vector2f& size);
        void snapToTarget();
        bool isSettled() const;

        void setCardPosition(CardPos pos);
        void setSelected(bool selected);
        bool isSelected() const { return selected; }

        void setExpanded(bool expanded);
        bool isExpanded() const { return expanded; }

        void setOpacity(float alpha);

        // Getters
        sf::Vector2f getSize() const { return currentSize; }
        sf::Vector2f getPosition() const { return currentPos; }
        sf::FloatRect getGlobalBounds() const;
        const sf::Text& getTitleText() const { return textTitle; }
        int getId() const { return config.id; }
        sf::Color getThemeColor() const { return config.themeColor; }
//        const sf::Text& getTitleText() const;

        // Callbacks
        std::function<void()> onSelect;
        std::function<void()> onViewMore;
        std::function<void()> onStart;
        std::function<void()> onBack;

    private:
        void updateLayout(float selectT, float expandT);

        // [2] QUAN TRỌNG: Phải khai báo biến config ở đây thì .cpp mới dùng được
        CardConfig config;

        bool selected;
        bool expanded;
        CardPos cardPos;

        // Visual Components
        GUI::Squircle bgShape;
        sf::Text textNumber;
        sf::Text textShortDesc; // [MỚI] Thêm text này
        sf::Text textTitle;
        sf::Sprite iconSprite;
        float textureWidth;

        // Expanded Components
        sf::Text textBigTitle;
//        sf::Text textBigTitle;
        sf::Text textDescription; // Thêm dòng này
        Button* btnViewMore;
        Button* btnStart;
        Button* btnBack;

        // Physics
        Utils::Physics::Spring selectionSpring;
        Utils::Physics::Spring expansionSpring;

        sf::Vector2f currentPos, targetPos;
        sf::Vector2f currentSize, targetSize;

        float globalAlpha;
    };
}
