#include "NodeUI.h"
#include <string>

namespace GUI
{
    NodeUI::NodeUI(sf::Font* font, float radius)
    {
        m_font = font;

        // Trong Constructor của NodeUI.cpp, khởi tạo bằng 0:
        m_arcOffset = {0.0f, 0.0f};

        // Cài đặt hình dáng cơ bản
        m_shape.setRadius(radius);
        m_shape.setOrigin(radius, radius); // Đặt tâm gốc (Origin) vào chính giữa hình tròn
        m_shape.setOutlineThickness(2.0f);

        // Cài đặt chữ
        m_valueText.setFont(*m_font);
        m_valueText.setCharacterSize(20);

        // Khởi tạo màu sắc mặc định (Trắng viền xám, chữ đen)
        m_currentColor = m_targetColor = sf::Color::White;
        m_currentTextColor = m_targetTextColor = sf::Color::Black;
        m_currentOutlineColor = m_targetOutlineColor = sf::Color(200, 200, 200);

        m_shape.setFillColor(m_currentColor);
        m_shape.setOutlineColor(m_currentOutlineColor);
        m_valueText.setFillColor(m_currentTextColor);

        // Khởi tạo tỷ lệ scale là 1.0 (Kích thước thật)
        m_scaleSpring.position = 1.0f;
        m_scaleSpring.target = 1.0f;
    }

    // Implement 2 hàm get/set:
    void NodeUI::setArcOffset(sf::Vector2f offset)
    {
        m_arcOffset = offset;
    }

    sf::Vector2f NodeUI::getArcOffset() const
    {
        return m_arcOffset;
    }

    void NodeUI::applyState(const Core::NodeState& state)
    {
        // 1. Áp dụng ngay lập tức vị trí hoàn hảo từ Timeline
        m_currentPosition = state.position;
        m_shape.setPosition(m_currentPosition);
        m_valueText.setPosition(m_currentPosition);

        // 2. Cập nhật các trạng thái màu sắc và Text
//        setValue(std::to_string(state.value));
        setValue(state.value);

        m_scaleSpring.target = state.scale;
//        m_targetColor = state.fillColor;
//        m_targetOutlineColor = state.outlineColor;
//        m_targetTextColor = state.textColor;

        setTargetColor(state.fillColor, state.textColor, state.outlineColor);

        // Cập nhật arcOffset nếu cần cho việc vẽ cây
        m_arcOffset = state.arcPivot;
    }

    void NodeUI::setValue(const std::string& val)
    {
        m_valueText.setString(val);

        // Thuật toán căn giữa tuyệt đối cho sf::Text dựa trên kích thước thực của chuỗi
        sf::FloatRect textRect = m_valueText.getLocalBounds();
        m_valueText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    }

    void NodeUI::setTargetPosition(float x, float y)
    {
        // Chỉ cập nhật Target, lò xo sẽ tự tính toán lực kéo trong hàm update()
//        m_xSpring.target = x;
//        m_ySpring.target = y;
    }

    void NodeUI::setExactPosition(float x, float y)
    {
//        // Ép vị trí hiện tại và đích đến bằng nhau
//        m_xSpring.position = x;
//        m_xSpring.target = x;
//        m_xSpring.velocity = 0.0f; // Triệt tiêu vận tốc dư thừa để Node không bị nảy
//
//        m_ySpring.position = y;
//        m_ySpring.target = y;
//        m_ySpring.velocity = 0.0f;
//
//        m_shape.setPosition(x, y);
//        m_valueText.setPosition(x, y);
    }

    void NodeUI::setTargetColor(sf::Color bgTarget, sf::Color textTarget, sf::Color outlineTarget)
    {
        m_targetColor = bgTarget;
        m_targetTextColor = textTarget;
        m_targetOutlineColor = outlineTarget;
    }

    void NodeUI::setExactColor(sf::Color bg, sf::Color text, sf::Color outline)
    {
        m_currentColor = m_targetColor = bg;
        m_currentTextColor = m_targetTextColor = text;
        m_currentOutlineColor = m_targetOutlineColor = outline;

        m_shape.setFillColor(m_currentColor);
        m_valueText.setFillColor(m_currentTextColor);
        m_shape.setOutlineColor(m_currentOutlineColor);
    }

    void NodeUI::setTargetScale(float scale)
    {
        m_scaleSpring.target = scale;
    }

    sf::Vector2f NodeUI::getCurrentPosition() const
    {
        // Trả về vị trí đang trượt của lò xo, rất quan trọng để EdgeUI bám theo
//        return sf::Vector2f(m_xSpring.position, m_ySpring.position);
        return m_currentPosition;
    }

    sf::Vector2f NodeUI::getVelocity() const
    {
        return m_velocity;
//        return sf::Vector2f(m_xSpring.velocity, m_ySpring.velocity);
    }

    sf::Vector2f NodeUI::getTargetPosition() const
    {
        return m_currentPosition;
//        return sf::Vector2f(m_xSpring.target, m_ySpring.target);
    }

    void NodeUI::update(float dt)
    {
        // 1. TÍNH TOÁN VẬN TỐC (Cực kỳ quan trọng cho EdgeUI)
        if(dt > 0.0001f)
        {
            // Vận tốc = (Vị trí hiện tại - Vị trí trước đó) / Thời gian
            m_velocity = (m_currentPosition - m_lastPosition) / dt;
            m_lastPosition = m_currentPosition;
        }

        // 1. Cập nhật vật lý cho lò xo (độ lớn)
//        m_xSpring.update(dt);
//        m_ySpring.update(dt);
        m_scaleSpring.update(dt);

        // 2. Áp dụng tọa độ mới vào hình khối và chữ
//        m_shape.setPosition(m_xSpring.position, m_ySpring.position);
//        m_valueText.setPosition(m_xSpring.position, m_ySpring.position);
        m_shape.setPosition(m_currentPosition);
        m_valueText.setPosition(m_currentPosition);

        m_shape.setScale(m_scaleSpring.position, m_scaleSpring.position);
        m_valueText.setScale(m_scaleSpring.position, m_scaleSpring.position);

        // 3. Tính toán nội suy màu sắc mượt mà (Hệ số 10.0f * dt là tốc độ chuyển màu)
        m_currentColor = Utils::Math::Easing::lerpColor(m_currentColor, m_targetColor, 10.0f * dt);
        m_currentTextColor = Utils::Math::Easing::lerpColor(m_currentTextColor, m_targetTextColor, 10.0f * dt);
        m_currentOutlineColor = Utils::Math::Easing::lerpColor(m_currentOutlineColor, m_targetOutlineColor, 10.0f * dt);

        // 4. Áp dụng màu sắc mới vào hình khối và chữ
        m_shape.setFillColor(m_currentColor);
        m_valueText.setFillColor(m_currentTextColor);
        m_shape.setOutlineColor(m_currentOutlineColor);
    }

    void NodeUI::draw(sf::RenderTarget& target) const
    {
        target.draw(m_shape);
        target.draw(m_valueText);
    }
}
