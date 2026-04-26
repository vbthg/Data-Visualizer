#include "NodeUI.h"
#include <string>
#include <iostream>

namespace GUI
{
    NodeUI::NodeUI(sf::Font* font, float radius, const Core::NodeState* initialState)
    {
        m_font = font;
        m_arcOffset = {0.0f, 0.0f};

        // 1. THIẾT LẬP CƠ BẢN (SHAPE & TEXT CONFIG)
        m_shape.setRadius(radius);
        m_shape.setOrigin(radius, radius);
        m_shape.setOutlineThickness(2.0f);

        m_valueText.setFont(*m_font);
        m_valueText.setCharacterSize(20);

        m_subText.setFont(*m_font);
        m_subText.setCharacterSize(14);

        // 2. LOGIC KHỞI TẠO TRẠNG THÁI (TRÁNH FLICKER)
        if(initialState)
        {
            // Đồng bộ dữ liệu logic
            m_currentPosition = initialState->position;
            m_lastPosition = m_currentPosition; // Vận tốc ban đầu = 0
            m_opacity = initialState->opacity;

            // Tận dụng hàm setValue để căn giữa text chính
            setValue(initialState->value);

            // Đồng bộ subText
            m_subText.setString(initialState->subText);
            sf::FloatRect sBounds = m_subText.getLocalBounds();
            m_subText.setOrigin(sBounds.left + sBounds.width / 2.0f, sBounds.top + sBounds.height / 2.0f);

            // Ép vật lý lò xo về đúng giá trị snapshot
            m_scaleSpring.snapTo(initialState->scale);

            // Thiết lập màu sắc (Gán thẳng currentColor để không bị lerp từ trắng sang)
            m_currentColor = m_targetColor = initialState->fillColor;
            m_currentTextColor = m_targetTextColor = initialState->textColor;
            m_currentOutlineColor = m_targetOutlineColor = initialState->outlineColor;

            m_isDraggable = initialState->isDraggable;

            // ĐỒNG BỘ TRỰC TIẾP LÊN SFML OBJECTS
            // Quan trọng: Phải set trước khi hàm draw() đầu tiên có cơ hội chạy
            m_shape.setPosition(m_currentPosition);
            m_shape.setScale(m_scaleSpring.position, m_scaleSpring.position);

            m_valueText.setPosition(m_currentPosition);
            m_valueText.setScale(m_scaleSpring.position, m_scaleSpring.position);

            m_subText.setPosition(m_currentPosition.x, m_currentPosition.y - radius - 20.f);

            m_shape.setFillColor(applyAlpha(m_currentColor, m_opacity));
            m_shape.setOutlineColor(applyAlpha(m_currentOutlineColor, m_opacity));
            m_valueText.setFillColor(applyAlpha(m_currentTextColor, m_opacity));
            m_subText.setFillColor(applyAlpha(m_currentTextColor, m_opacity));
        }
        else
        {
            // Trạng thái mặc định nếu không có initialState truyền vào
            m_currentPosition = {0.0f, 0.0f};
            m_lastPosition = {0.0f, 0.0f};
            m_opacity = 1.0f;
            m_scaleSpring.snapTo(1.0f);

            m_currentColor = m_targetColor = sf::Color::White;
            m_currentTextColor = m_targetTextColor = sf::Color::Black;
            m_currentOutlineColor = m_targetOutlineColor = sf::Color(200, 200, 200);

            m_shape.setFillColor(m_currentColor);
            m_shape.setOutlineColor(m_currentOutlineColor);
            m_valueText.setFillColor(m_currentTextColor);
            m_subText.setFillColor(applyAlpha(m_currentTextColor, 0.6f));

            m_isDraggable = false;
        }

        m_isDragging = false;
        m_dragOffset = {0.0f, 0.0f};
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

    void NodeUI::setDraggable(bool enabled)
    {
        m_isDraggable = enabled;
        if(!enabled)
        {
            m_isDragging = false; // Nếu tắt tính năng khi đang kéo thì dừng ngay
        }
    }

    bool NodeUI::isDraggable() const
    {
        return m_isDraggable;
    }

    bool NodeUI::isDragging() const
    {
        return m_isDragging;
    }

    void NodeUI::forceRelease()
    {
        m_isDragging = false;
    }

    void NodeUI::applyState(const Core::NodeState& state)
    {
        m_currentPosition = state.position;
        m_shape.setPosition(m_currentPosition);
        m_valueText.setPosition(m_currentPosition);

        setValue(state.value);

        // XỬ LÝ SUBTEXT
        m_subText.setString(state.subText);
        sf::FloatRect sBounds = m_subText.getLocalBounds();
        m_subText.setOrigin(sBounds.left + sBounds.width / 2.0f, sBounds.top + sBounds.height / 2.0f);
        // Đặt subText cách tâm Node một khoảng (radius + 20px) lên phía trên
        m_subText.setPosition(m_currentPosition.x, m_currentPosition.y - m_shape.getRadius() - 20.f);

//        m_scaleSpring.target = state.scale;
        m_scaleSpring.snapTo(state.scale);
        setTargetColor(state.fillColor, state.textColor, state.outlineColor);

        m_arcOffset = state.arcPivot;
        m_isDraggable = state.isDraggable;
        m_opacity = state.opacity; // Áp dụng opacity từ state
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

    bool NodeUI::handleEvent(const sf::Event& event, sf::Vector2f worldPos)
    {
        if(!m_isDraggable) return false;

        if(event.type == sf::Event::MouseButtonPressed)
        {
            if(event.mouseButton.button == sf::Mouse::Left)
            {
                float dx = worldPos.x - m_currentPosition.x;
                float dy = worldPos.y - m_currentPosition.y;
                float radius = m_shape.getRadius();

                if((dx * dx + dy * dy) <= (radius * radius))
                {
                    m_isDragging = true;
                    m_dragOffset = m_currentPosition - worldPos;
                    return true; // Báo cáo: "Tôi đã nhận Node này, đừng đưa cho ai khác!"
                }
            }
        }
        else if(event.type == sf::Event::MouseButtonReleased)
        {
            if(event.mouseButton.button == sf::Mouse::Left)
            {
                m_isDragging = false;
            }
        }
        return false;
    }

    sf::Color NodeUI::applyAlpha(const sf::Color& color, float alphaMult)
    {
        sf::Color res = color;
        res.a = color.a * alphaMult;
        return res;
    }

    void NodeUI::update(float dt, sf::Vector2f worldPos)
    {
        if(m_isDragging)
        {
            m_currentPosition = worldPos + m_dragOffset;
        }

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

        m_subText.setPosition(m_currentPosition.x, m_currentPosition.y - m_shape.getRadius() - 20.f);

        m_shape.setPosition(m_currentPosition);
        m_valueText.setPosition(m_currentPosition);

        m_shape.setScale(m_scaleSpring.position, m_scaleSpring.position);
        m_valueText.setScale(m_scaleSpring.position, m_scaleSpring.position);

        // 3. Tính toán nội suy màu sắc mượt mà (Hệ số 10.0f * dt là tốc độ chuyển màu)
        sf::Uint8 alpha = static_cast<sf::Uint8>(m_opacity * 255.f);
        m_currentColor = Utils::Math::Easing::lerpColor(m_currentColor, m_targetColor, 10.0f * dt);
        m_currentTextColor = Utils::Math::Easing::lerpColor(m_currentTextColor, m_targetTextColor, 10.0f * dt);
        m_currentOutlineColor = Utils::Math::Easing::lerpColor(m_currentOutlineColor, m_targetOutlineColor, 10.0f * dt);

        // 4. Áp dụng màu sắc mới vào hình khối và chữ
        m_shape.setFillColor(applyAlpha(m_currentColor, m_opacity));
        m_valueText.setFillColor(applyAlpha(m_currentTextColor, m_opacity));
        m_shape.setOutlineColor(applyAlpha(m_currentOutlineColor, m_opacity));
        m_subText.setFillColor(applyAlpha(m_currentTextColor, m_opacity));
    }

    void NodeUI::draw(sf::RenderTarget& target) const
    {
//        sf::String s = m_valueText.getString();
//        std::string ss = s.toAnsiString();
//
//        if(s == "3") std::cout << ss << " " << m_shape.getScale().x << " " << m_shape.getScale().y << "\n";

        target.draw(m_shape);
        target.draw(m_valueText);
        if(!m_subText.getString().isEmpty())
        {
            target.draw(m_subText);
        }
    }
}
