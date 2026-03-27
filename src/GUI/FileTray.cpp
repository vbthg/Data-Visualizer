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

    void FileTray::initElements(const std::string& icon, const std::string& text)
    {
        // Cấu hình Background
        m_trayBackground.setSize({90.f, 90.f});
        m_trayBackground.setRadius(24.f);
        m_trayBackground.setOrigin(45.f, 45.f);
        m_trayBackground.setFillColor(sf::Color(255, 255, 255, 60));

        // Cấu hình Icon & Title (Dùng font từ ResourceManager của bạn)
        m_trayIcon.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));
        m_trayIcon.setString(icon);
        m_trayIcon.setCharacterSize(30);
        centerOrigin(m_trayIcon);

        m_trayTitle.setString(text);
        m_trayTitle.setCharacterSize(12);
        centerOrigin(m_trayTitle);

        m_scaleSpring.snapTo(0.5f);
        m_scaleSpring.target = 1.0f;
    }

    void FileTray::update(float dt, sf::Vector2f notchSize)
    {
        std::cout << "[TRAY_UPDATE] Tray o dia chi " << this
              << " dang nhan Alpha = " << m_alpha
              << " va Offset = " << m_verticalOffset << std::endl;

        m_alpha = 1.f;
        m_verticalOffset = 0;

        m_scaleSpring.update(dt);

        float leftX = 65.0f;
        // verticalOffset lấy từ lớp cha NotchContent (do NotchManager điều khiển animation)
        float centerY = (notchSize.y / 2.0f) + m_verticalOffset;

        m_trayBackground.setPosition(leftX, centerY);
        m_trayBackground.setScale(m_scaleSpring.position, m_scaleSpring.position);

        m_trayIcon.setPosition(leftX, centerY - 12.f);
        m_trayTitle.setPosition(leftX, centerY + 22.f);
    }

    void FileTray::setAlpha(float alpha)
    {
        m_alpha = alpha; // Lưu lại để dùng trong hàm draw nếu cần

        // Áp dụng alpha cho từng thành phần local
        sf::Color c = m_trayIcon.getFillColor();
        c.a = static_cast<sf::Uint8>(alpha * 255.f);
        m_trayIcon.setFillColor(c);
        m_trayTitle.setFillColor(c);

        sf::Color bgC = m_trayBackground.getFillColor();
        bgC.a = static_cast<sf::Uint8>(alpha * 60.f);
        m_trayBackground.setFillColor(bgC);
    }


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
        target.draw(m_trayIcon, states);
        target.draw(m_trayTitle, states);

        sf::Color trayColor = m_trayBackground.getFillColor();
//        std::cout << (int)trayColor.r << " " << (int)trayColor.g << " " << (int)trayColor.b << " " << (int)trayColor.a << "\n";
    }
}
