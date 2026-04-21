#pragma once
#include "NotchContent.h"
#include "InputField.h" // Ta lây Squircle từ InputField hoặc theme
#include <functional>

namespace GUI {
    class FileTrayContent : public NotchContent {
    private:
        // CÁI HỘP DROP ZONE
        GUI::Squircle m_dropZoneContainer;

        sf::Text m_bigIcon;
        sf::Text m_mainLabel;
        sf::Text m_subtitle;
        std::function<void(std::string)> m_onFileDropped;

    public:
        FileTrayContent(const sf::Font& font);

        void setOnFileDropped(std::function<void(std::string)> cb) { m_onFileDropped = cb; }
        void handleFileDrop(const std::string& path);

        void update(float dt, sf::Vector2f notchSize) override;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setAlpha(float alpha) override;

        // Hàm này sau này sẽ dùng để vẽ nét đứt (Dashed line)
        void setHoverState(bool isHovered);
    };
}
