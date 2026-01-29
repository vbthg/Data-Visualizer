#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"

namespace GUI
{
    class TitleBar
    {
    public:
        // SỬA: Constructor nhận thêm window để tự quản lý
        TitleBar(sf::RenderWindow& window, const sf::Font& iconFont);

        void update(float dt); // Không cần truyền window nữa vì đã lưu rồi
        void handleEvent(const sf::Event& event); // Tự xử lý dragging luôn
        void draw(); // Không cần truyền window

        bool isFullscreen; // Để main biết mà vẽ layout khác (nếu cần)

        void toggleFullscreen();

    private:
        sf::RenderWindow& windowRef; // Tham chiếu tới cửa sổ chính

        const float HEIGHT = 32.0f;
        const float BTN_WIDTH = 46.0f;

        // Animation Vars
        float currentY;
        float velocityY;
        float targetY;
        bool isVisible;

        sf::RectangleShape barBackground;

        // Buttons
        GUI::Button btnClose;
        GUI::Button btnMaximize;
        GUI::Button btnMinimize;

        // Window Restoration Vars (Để lưu vị trí trước khi fullscreen)
        sf::Vector2i lastPos;
        sf::Vector2u lastSize;

//        sf::RenderWindow& windowRef;

//        // Window Restoration Vars
//        sf::Vector2i lastPos;
//        sf::Vector2u lastSize;

        // Internal Helpers

        void minimize();
        void updateMaximizeIcon();

        // Dragging Logic
        bool isDragging;
        sf::Vector2i grabOffset;
    };
}
