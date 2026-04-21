#pragma once
#include "NotchContent.h"
#include "InputField.h"
#include <functional>

namespace GUI
{
    class InputContent : public NotchContent
    {
    private:
        InputField m_input;
        std::function<void(std::string)> m_onPathSubmit;
        std::function<void()> m_onCancel;

        // Tỷ lệ bắt đầu hiện chữ (0.5 = Notch dãn được một nửa)
        const float TEXT_REVEAL_THRESHOLD = 0.5f;

    public:
        InputContent(const sf::Font& font);

        // Chuyển tiếp sự kiện từ NotchManager vào đây
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

        // Cập nhật và vẽ
        void update(float dt, sf::Vector2f notchSize) override;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Điều khiển độ trong suốt (Morphing logic)
        void setAlpha(float alpha) override;

        // Callbacks
        void setOnSubmit(std::function<void(std::string)> cb) { m_onPathSubmit = cb; }
        void setOnCancel(std::function<void()> cb) { m_onCancel = cb; }

        // Tiện ích
        void forceFocus(bool focus) { m_input.setFocus(focus); }
    };
}
