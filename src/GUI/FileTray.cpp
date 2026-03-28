#include "FileTray.h"
#include "ResourceManager.h"
#include <cmath>

namespace GUI
{
//    FileTray::FileTray(const std::string& iconCode, const std::string& title)
//        : NotchContent(iconCode, title, "")
//    {
//        // Khởi tạo nền Squircle
//        m_background.setSize(sf::Vector2f(90.f, 90.f));
//        m_background.setPower(2.0f);
//        m_background.setRadius(24.f);
//        m_background.setFillColor(sf::Color(255, 255, 255, 15));
//
//        // Đặt Origin ở giữa để khi Spring co giãn sẽ scale từ tâm
//        m_background.setOrigin(45.f, 45.f);
//
//        m_icon.setCharacterSize(36);
//        centerOrigin(m_icon);
//
//        m_title.setCharacterSize(14);
//        centerOrigin(m_title);
//
//        // Cấu hình vật lý lò xo
//        m_scaleSpring.stiffness = 500.f;
//        m_scaleSpring.damping = 25.f;
//        m_scaleSpring.snapTo(0.5f); // Bắt đầu ở kích thước bằng một nửa
//        m_scaleSpring.target = 1.0f; // Bật lên kích thước thật
//    }

    void FileTray::initElements(const sf::String& icon, const sf::String& title, const sf::String& subtitle)
    {
        // Cấu hình Background
        m_trayBackground.setSize({255.f, 90.f});
        m_trayBackground.setRadius(38.f);
        m_trayBackground.setOrigin(45.f, 45.f);
//        m_trayBackground.setPower(2.f);
//        m_trayBackground.setFillColor(sf::Color(255, 255, 255, 60));
        m_trayBackground.setFillColor(sf::Color(255, 255, 255, 0));
//        m_trayBackground.setOutlineColor(sf::Color(255, 255, 255, 80));
        m_trayBackground.setOutlineColor(sf::Color(10, 132, 255, 255));
        m_trayBackground.setOutlineThickness(1.f);

        // Cấu hình Icon & Title (Dùng font từ ResourceManager của bạn)
        m_trayIcon.setFont(ResourceManager::getInstance().getFont("assets/fonts/Phosphor.ttf"));
        m_trayIcon.setString(icon);
//        m_trayIcon.setString(icon);
        m_trayIcon.setCharacterSize(30);
        centerOrigin(m_trayIcon);

        m_trayTitle.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));
        m_trayTitle.setString(title);
        m_trayTitle.setCharacterSize(17);
        m_trayTitle.setFillColor(sf::Color(10, 132, 255, 255));
        centerOrigin(m_trayTitle);

        m_traySubtitle.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));
        m_traySubtitle.setString(subtitle);
        m_traySubtitle.setCharacterSize(12);
        m_traySubtitle.setFillColor(sf::Color(10, 132, 255, 255));
        centerOrigin(m_traySubtitle);

        m_scaleSpring.stiffness = 650.f;
        m_scaleSpring.damping = 35.f;
        m_scaleSpring.snapTo(0.5f);
        m_scaleSpring.target = 1.0f;
    }

    void FileTray::update(float dt, sf::Vector2f notchSize)
    {
        std::cout << "[TRAY_UPDATE] Tray o dia chi " << this
              << " dang nhan Alpha = " << m_alpha
              << " va Offset = " << m_verticalOffset << std::endl;

//        m_alpha = 1.f;
//        m_verticalOffset = 0;

        m_scaleSpring.update(dt);

        float leftX = 55.0f;
        // verticalOffset lấy từ lớp cha NotchContent (do NotchManager điều khiển animation)
        float centerY = (notchSize.y / 2.0f) + m_verticalOffset + 5.f;

        m_trayBackground.setPosition(leftX, centerY);
        m_trayBackground.setScale(m_scaleSpring.position, m_scaleSpring.position);

        m_trayIcon.setScale(m_scaleSpring.position, m_scaleSpring.position);
        m_trayIcon.setPosition(leftX, centerY - 12.f);
        m_trayTitle.setScale(m_scaleSpring.position, m_scaleSpring.position);
        m_trayTitle.setPosition(leftX + 30.f, centerY + 22.f);
        m_traySubtitle.setScale(m_scaleSpring.position, m_scaleSpring.position);
        m_traySubtitle.setPosition(leftX + 50.f, centerY + 37.f);
    }

    void FileTray::setAlpha(float alpha)
    {
        m_alpha = alpha; // Lưu lại để dùng trong hàm draw nếu cần

//        std::cout << m_alpha << "\n";

        // Áp dụng alpha cho từng thành phần local
        sf::Color c = m_trayTitle.getFillColor();
        c.a = static_cast<sf::Uint8>(alpha * 255.f);
        m_trayIcon.setFillColor(c);
        m_trayTitle.setFillColor(c);

//        sf::Color bgC = m_trayBackground.getFillColor();
        sf::Color bgC = sf::Color(10, 132, 255, 255);
        bgC.a = static_cast<sf::Uint8>(alpha * 255.f);
        m_trayBackground.setOutlineColor(bgC);
    }

    void FileTray::setScale(float x) { m_scaleSpring.target = x; }


    void FileTray::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if(m_alpha <= 0.01f) return;
//        m_alpha = 255.f;
//        std::cout << "m_alpha = " << m_alpha << "\n";

        states.transform *= getTransform();

        // VẼ THỬ MỘT CÁI HÌNH TRÒN ĐỎ CHÓI TẠI VỊ TRÍ SQUIRCLE
//        sf::CircleShape debugCircle(20.f);
//        debugCircle.setFillColor(sf::Color::Red);
//        debugCircle.setOrigin(20.f, 20.f);
//        debugCircle.setPosition(m_trayBackground.getPosition()); // Lấy tọa độ của Squircle
//        sf::Color cl = debugCircle.s

//        target.draw(debugCircle, states); // Vẽ cái này trước

        target.draw(m_trayBackground, states);

        sf::RenderStates iconStates = states;
        iconStates.shader = nullptr;
//        barStates.transform.translate(25.f, m_size.y - m_progressBar.getSize().y - 5.f); // Dịch lên 10.0f để tránh bị che khuất ở mép dưới
        target.draw(m_trayIcon, iconStates);
        target.draw(m_trayTitle, iconStates);

        target.draw(m_trayIcon, states);
        target.draw(m_trayTitle, states);
        target.draw(m_traySubtitle, states);

        sf::Color trayColor = m_trayBackground.getFillColor();
//        std::cout << (int)trayColor.r << " " << (int)trayColor.g << " " << (int)trayColor.b << " " << (int)trayColor.a << "\n";
    }
}
