#include "InputContent.h"

namespace GUI
{
    InputContent::InputContent(const sf::Font& font)
        : NotchContent("", "", ""), // Cha không cần quản lý text
          m_input(font, InputField::Type::String, {560.f, 34.f})
    {
        m_input.setPlaceholder("Enter path to file...");
        m_input.setFocus(true); // Tự động focus ngay khi hiện ra
    }

    void InputContent::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        m_input.handleEvent(event, window);

        if (event.type == sf::Event::KeyPressed)
        {
            // Enter để nộp đường dẫn
            if (event.key.code == sf::Keyboard::Enter)
            {
                if (m_onPathSubmit && !m_input.getText().empty())
                {
                    m_onPathSubmit(m_input.getText());
                }
                else
                {
                    m_input.triggerErrorShake();
                }
            }
            // Escape để hủy bỏ, quay lại ban đầu
            else if (event.key.code == sf::Keyboard::Escape)
            {
                if (m_onCancel) m_onCancel();
            }
        }
    }

    void InputContent::update(float dt, sf::Vector2f notchSize)
    {
        // Luôn giữ InputField ở tâm của Notch đang biến hình
        m_input.setPosition({notchSize.x / 2.f, notchSize.y / 2.f});
        m_input.update(dt);
    }

    void InputContent::setAlpha(float alpha)
    {
        m_alpha = alpha; // Biến của NotchContent

        // Logic Delayed Reveal:
        // Chữ và con trỏ sẽ tàng hình cho đến khi Notch dãn được 50%
        float textAlpha = 0.f;
        if (alpha > TEXT_REVEAL_THRESHOLD)
        {
            // Remap alpha từ [0.5, 1.0] thành [0.0, 1.0] để fade-in mượt
            textAlpha = (alpha - TEXT_REVEAL_THRESHOLD) / (1.0f - TEXT_REVEAL_THRESHOLD);
        }

        // THAY ĐỔI Ở ĐÂY: Không gọi setFillColor với alpha thấp nữa
        // Thay vào đó, ta gọi một hàm mới trong InputField để đồng bộ độ mờ của toàn bộ linh kiện
        m_input.setGlobalAlpha(textAlpha);

//        // 1. Chỉnh màu nền (Glassmorphism)
//        // Dùng màu trắng mờ, tăng dần theo Notch
//        sf::Color bgCol = sf::Color(255, 255, 255, (sf::Uint8)(alpha * 30.f));
//        m_input.setFillColor(bgCol);
//
//        // 2. Chỉnh màu viền và chữ (Chỉ hiện khi đủ độ trễ)
//        sf::Color borderCol = sf::Color(255, 255, 255, (sf::Uint8)(textAlpha * 80.f));
//        m_input.setOutlineColor(borderCol);
//
//        // Lưu ý: Ông cần thêm hàm setTextAlpha vào InputField
//        // hoặc chỉnh sửa trực tiếp màu text/placeholder ở đây nếu cần.
    }

    void InputContent::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Không vẽ nếu quá mờ
        if (m_alpha < 0.05f) return;

        states.transform *= getTransform();
        target.draw(m_input, states);
    }
}
