#include "FileTrayContent.h"
#include "ResourceManager.h"
#include "Theme.h"

namespace GUI {
    FileTrayContent::FileTrayContent(const sf::Font& font)
        : NotchContent("", "", ""), // Cha không cần quản lý text
          m_bigIcon("", font, 30), m_mainLabel("", font, 16),
          m_subtitle("Drop a file here", font, 12)
    {
        // 1. Setup CÁI HỘP DROPZONE (Lồng vào trong Notch 320x170)
        m_dropZoneContainer.setPower(4.f); // Squircle power 3
        m_dropZoneContainer.setSize({280.f, 130.f}); // Chừa lề 20px mỗi bên
        m_dropZoneContainer.setRadius(50.f);
        m_dropZoneContainer.setOrigin({140.f, 65.f}); // Center origin
        m_dropZoneContainer.setFillColor(sf::Color(255, 255, 255, 15)); // Trắng mờ 5%
        m_dropZoneContainer.setOutlineThickness(1.5f);
        m_dropZoneContainer.setOutlineColor(sf::Color(255, 255, 255, 50)); // Viền trắng mờ 20%

        // 2. Setup Nội dung bên trong hộp
        auto& res = ResourceManager::getInstance();
        m_bigIcon.setFont(res.getFont("assets/fonts/Phosphor.ttf"));
        m_bigIcon.setString(L"\uE4AA"); // Icon file/cloud
        m_mainLabel.setStyle(sf::Text::Bold);
        m_subtitle.setFillColor(sf::Color(180, 180, 180));

        // Center all origin
        auto centerTxt = [](sf::Text& text) {
            sf::FloatRect b = text.getLocalBounds();
            text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        };
        centerTxt(m_bigIcon); centerTxt(m_mainLabel); centerTxt(m_subtitle);
    }

    void FileTrayContent::handleFileDrop(const std::string& path) {
        if (m_onFileDropped) m_onFileDropped(path);
    }

    void FileTrayContent::update(float dt, sf::Vector2f notchSize) {
        float cx = notchSize.x / 2.f;
        float cy = notchSize.y / 2.f + m_verticalOffset;

        // Bố cục lồng nhau: Hộp nằm giữa, nội dung nằm trong hộp
        m_dropZoneContainer.setPosition(cx, cy);

        m_bigIcon.setPosition(cx, cy - 20.f);
        m_mainLabel.setPosition(cx, cy + 25.f);
        m_subtitle.setPosition(cx, cy + 25.f);
    }

    void FileTrayContent::setAlpha(float alpha) {
        m_alpha = alpha; // Alpha của Notch transition

        // Logic Delayed Reveal:
        // Hộp và chữ chỉ hiện khi Notch dãn được 60% (để tạo cảm giác nở ra)
        float revealAlpha = 0.f;
        if (alpha > 0.6f) {
            revealAlpha = (alpha - 0.6f) / 0.4f; // Remap sang dải 0-1
        }

        sf::Uint8 a = static_cast<sf::Uint8>(revealAlpha * 255.f);

        // Chỉnh màu hộp DropZone (Tăng độ trong suốt theo morph)
        m_dropZoneContainer.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(revealAlpha * 15.f)));
        m_dropZoneContainer.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(revealAlpha * 50.f)));

        // Chỉnh màu chữ
        m_bigIcon.setFillColor(sf::Color(255, 255, 255, a));
        m_mainLabel.setFillColor(sf::Color(255, 255, 255, a));
        m_subtitle.setFillColor(sf::Color(180, 180, 180, a));
    }

    void FileTrayContent::setHoverState(bool isHovered) {
        // Tương lai ông sẽ viết logic vẽ Dashed Line ở đây
        if (isHovered) {
            m_dropZoneContainer.setOutlineColor(sf::Color::Cyan); // Viền Cyan cho nổi
            m_bigIcon.setScale(1.1f, 1.1f); // Phóng to icon
        } else {
            m_dropZoneContainer.setOutlineColor(sf::Color(255, 255, 255, 50));
            m_bigIcon.setScale(1.0f, 1.0f);
        }
    }

    void FileTrayContent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Không vẽ nếu quá mờ
        if (m_alpha < 0.05f) return;

        states.transform *= getTransform();

        // Vẽ CÁI HỘP trước
        target.draw(m_dropZoneContainer, states);

        // Vẽ nội dung sau
        target.draw(m_bigIcon, states);
        target.draw(m_mainLabel, states);
        target.draw(m_subtitle, states);
    }
}
