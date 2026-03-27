#pragma once
#include <SFML/Graphics.hpp>
#include <string>
//#include "GUI/Waveform.h"

namespace GUI
{
    class NotchContent : public sf::Drawable, public sf::Transformable
    {
    public:
        NotchContent(const std::string& iconCode, const std::string& title, const std::string& subtitle);
        virtual ~NotchContent() = default;

        // notchSize dùng để tính toán tọa độ Neo (Anchoring) cho 3 Zones
        virtual void update(float dt, sf::Vector2f notchSize);

        // Điều khiển độ mờ của toàn bộ cụm content (0.0f - 1.0f)
        virtual void setAlpha(float alpha);
        float getAlpha() const;

        // Trượt content theo trục Y để tạo hiệu ứng Fade-in/out Apple style
        void setVerticalOffset(float offset);

        void setTitle(const std::string& title);
        void setSubtitle(const std::string& subtitle);
        void setIcon(const std::string& iconCode);

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        sf::Text m_icon;
        sf::Text m_title;
        sf::Text m_subtitle;
        float m_alpha;
        float m_verticalOffset;
        float m_padding;

        void centerOrigin(sf::Text& text);

    private:

//        void centerOrigin(sf::Text& text); // Helper để tính lại tâm

//        sf::Text m_icon;
//        sf::Text m_title;
//        sf::Text m_subtitle;
//        GUI::Waveform m_waveform;

//        float m_alpha;
//        float m_verticalOffset;
//        float m_padding; // Khoảng cách từ mép Notch vào nội dung
    };
}
