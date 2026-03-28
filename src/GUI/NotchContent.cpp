#include "NotchContent.h"
#include "ResourceManager.h"
#include <math.h>

namespace GUI
{
    NotchContent::NotchContent(const std::string& iconCode, const std::string& title, const std::string& subtitle)
        : m_alpha(1.0f), m_verticalOffset(0.0f), m_padding(25.0f)
    {
        auto& res = ResourceManager::getInstance();

        m_icon.setFont(res.getFont("assets/fonts/Phosphor.ttf"));
        m_icon.setString(iconCode);
        m_icon.setCharacterSize(22);

        m_title.setFont(res.getFont("assets/fonts/SFProText-Regular.ttf"));
        m_title.setString(title);
        m_title.setCharacterSize(15);
        // Lưu ý: Cần set style Bold nếu font Regular không đủ đậm
        m_title.setStyle(sf::Text::Bold);

        m_subtitle.setFont(res.getFont("assets/fonts/SFProText-Regular.ttf"));
        m_subtitle.setString(subtitle);
        m_subtitle.setCharacterSize(12);
        m_subtitle.setFillColor(sf::Color(180, 180, 180));

        // Center alignment cho text
        auto centerText = [](sf::Text& t)
        {
            sf::FloatRect bounds = t.getLocalBounds();
            t.setOrigin(std::floor(bounds.left + bounds.width / 2.0f),
                       std::floor(bounds.top + bounds.height / 2.0f));
        };

        centerText(m_icon);
        centerText(m_title);
        centerText(m_subtitle);

//        setAlpha(1.f);
    }

    void NotchContent::setVerticalOffset(float offset)
    {
        m_verticalOffset = offset;
    }

    void NotchContent::centerOrigin(sf::Text& text)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        // Sử dụng floor để tránh hiện tượng chữ bị mờ do tọa độ lẻ (Sub-pixel rendering)
        text.setOrigin(std::floor(bounds.left + bounds.width / 2.0f),
                       std::floor(bounds.top + bounds.height / 2.0f));
    }

    void NotchContent::setTitle(const std::string& title)
    {
        m_title.setString(title);
        centerOrigin(m_title);
    }

    void NotchContent::setSubtitle(const std::string& subtitle)
    {
        m_subtitle.setString(subtitle);
        centerOrigin(m_subtitle);
    }

    void NotchContent::setIcon(const std::string& iconCode)
    {
        m_icon.setString(iconCode);
        centerOrigin(m_icon);
    }

    void NotchContent::update(float dt, sf::Vector2f notchSize)
    {
        float centerX = notchSize.x / 2.0f;
        float centerY = notchSize.y / 2.0f + m_verticalOffset;

        // Icon bám mép trái (x = 0 + padding)
        m_icon.setPosition(m_padding + 10.f, centerY);

        // Chữ nằm chính giữa Notch (x = centerX)
        m_title.setPosition(centerX, centerY - 9.f);
        m_subtitle.setPosition(centerX, centerY + 10.f);
    }

    void NotchContent::setAlpha(float alpha)
    {
        m_alpha = std::max(0.0f, std::min(1.0f, alpha));

        m_alpha = alpha;
        sf::Uint8 a = (sf::Uint8)(alpha * 255.f);

        m_icon.setFillColor(sf::Color(255, 255, 255, a));
        m_title.setFillColor(sf::Color(255, 255, 255, a));

        // Subtitle mặc định xám hơn
        sf::Color subCol = sf::Color(180, 180, 180, a);
        m_subtitle.setFillColor(subCol);

//        m_waveform.setAlpha(alpha);
    }

    void NotchContent::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Nếu độ trong suốt quá thấp (gần như tàng hình), bỏ qua luôn không thèm vẽ nữa
        if (m_alpha <= 0.01f) return;

//        m_title.setColor(sf::Color::Red);
//        m_subtitle.setColor(sf::Color::Red);

//        sf::Text title = m_title, subtitle = m_subtitle;
//        title.setColor(sf::Color::Red), subtitle.setColor(sf::Color::Red);

        states.transform *= getTransform();
        target.draw(m_icon, states);
        target.draw(m_title, states);
        target.draw(m_subtitle, states);
//        target.draw(m_waveform, states);
    }
}
