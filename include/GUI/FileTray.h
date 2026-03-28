#pragma once
#include "NotchContent.h"
#include "Spring.h" // Đường dẫn tới file Spring của bạn
#include "Squircle.h"
#include <iostream>
// Giả định bạn có class Squircle được định nghĩa sẵn dựa trên Waveform.cpp
// #include "Squircle.h"

namespace GUI
{
    class FileTray : public NotchContent
    {
    private:
        // Các thành phần riêng biệt, không đụng hàng
        GUI::Squircle m_trayBackground;
        sf::Text m_trayIcon;
        sf::Text m_trayTitle;
        sf::Text m_traySubtitle;

        Utils::Physics::Spring m_scaleSpring;

    public:
        // Truyền chuỗi rỗng vào lớp cha để nó không vẽ gì cả
        FileTray(const sf::String& iconCode, const sf::String& title, const sf::String& subtitle)
            : NotchContent("", "", "")
        {
            static int instanceCount = 0;
            instanceCount++;
            std::cout << "[LIFE] FileTray Instance #" << instanceCount << " created!" << std::endl;

            initElements(iconCode, title, subtitle);
        }

        void update(float dt, sf::Vector2f notchSize) override;
        void setAlpha(float alpha) override;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setScale(float x);

    private:
        void initElements(const sf::String& icon, const sf::String& title, const sf::String& subtitle);
    };
}
