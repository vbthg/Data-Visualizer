#pragma once
#include <SFML/Graphics.hpp>
#include <string>
//#include <queue>
#include "RoundedRectangleShape.h"
#include "Spring.h"

namespace GUI
{
    class DynamicIsland
    {
    public:
        DynamicIsland(const sf::Font& font);
        ~DynamicIsland();

        // Nạp thông báo vào hàng đợi
//        void pushMessage(const std::string& msg);

        void showMessage(const std::string& msg);
        void hideMessage(); // Lệnh ép mờ chữ cất đi

        // Dọn dẹp khẩn cấp (khi người dùng ấn Reset thuật toán)
        void clear();

        // Tâm neo của viên thuốc (Thường đặt ở giữa cạnh trên màn hình)
        void setPosition(float x, float y);

        // Cập nhật vị trí X ngay lập tức (Neo vào giữa màn hình)
        void setX(float x);

        // Các hàm điều khiển hiệu ứng trượt Y giống FloatingDock
        void setTargetY(float targetY);
        void snapToY(float startY);

        void update(float dt);
        void draw(sf::RenderWindow& window);

    private:
        enum class State
        {
            Idle,       // Nghỉ ngơi (Viên thuốc nhỏ)
            Expanding,  // Đang bung kích thước & Hiện chữ
//            Reading,    // Đứng im chờ người dùng đọc
            FadingOut   // Mờ chữ chuẩn bị đổi câu hoặc thu nhỏ
        };

        State currentState;
        sf::Vector2f position;

        // Thêm lò xo trục Y
        Utils::Physics::Spring ySpring;

        // --- Visuals ---
        RoundedRectangleShape background;
        sf::Text logText;

        // --- Data ---
//        std::queue<std::string> messageQueue;
        std::string currentMessage;

        // --- Timers & Physics ---
        float displayTimer;
        const float DisplayDuration = 1.5f; // Đứng im 1.5 giây mỗi câu

        Utils::Physics::Spring wSpring;
        Utils::Physics::Spring hSpring;
        Utils::Physics::Spring alphaSpring;

        // Tính toán kích thước đích dựa trên chuỗi hiện tại
        void calculateTargetBounds();
    };
}
