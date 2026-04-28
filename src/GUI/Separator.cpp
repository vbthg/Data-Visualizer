#include "Separator.h"
#include "Theme.h" // Nếu bạn có thông số chung trong Theme

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    Separator::Separator(float dockHeight)
    {
        m_width = 2.0f; // Rất mỏng
        float height = dockHeight * 0.6f; // Cao bằng 50% thanh Dock

        m_line.setSize({m_width, height});

        // Màu trắng với Alpha 12% tạo cảm giác vệt mờ xuyên thấu trên mặt kính
        m_line.setFillColor(sf::Color(255, 255, 255, 30));

        // Đặt tâm ở giữa để Dock dễ dàng căn chỉnh
        m_line.setOrigin(m_width / 2.0f, height / 2.0f);
    }

    void Separator::update(sf::RenderWindow& window, float dt)
    {
        // Vạch ngăn tĩnh, không cần update logic
    }

    void Separator::draw(sf::RenderTarget& target)
    {
        target.draw(m_line);
    }

    bool Separator::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        return false;
        // Vạch ngăn không nhận tương tác chuột
    }

    void Separator::setPosition(const sf::Vector2f pos)
    {
        m_position = pos;
        m_line.setPosition(pos);
    }

    sf::Vector2f Separator::getPosition() const
    {
        return m_position;
    }

    float Separator::getWidth() const
    {
        // Chiều rộng này báo cáo cho Dock để cộng dồn vào Target Width
        // Bạn có thể cộng thêm một chút margin (VD: m_width + 10.0f)
        // để tạo khoảng trống 2 bên vạch ngăn.
        return Theme::Style::SeparatorMargin;
    }
}
