#include "Slider.h"
#include "ResourceManager.h"

namespace GUI
{
    Slider::Slider(float trackWidth)
        : trackWidth(trackWidth), minValue(0.f), maxValue(1.f), stepSize(0.1f),
          currentValue(0.5f), isDragging(false), isHovered(false), opacityFactor(1.0f), position(0.f, 0.f)
    {
        // 1. Setup Track Nền
        bgTrack.setSize({trackWidth, trackHeight});
        bgTrack.setRadius(trackHeight / 2.0f); // Bo tròn hoàn toàn
        bgTrack.setPower(2.0f);
        bgTrack.setOrigin(0.0f, trackHeight / 2.0f);

        // Tùy theme của bạn, nền có thể là đen mờ hoặc trắng mờ. Ví dụ: đen mờ
        bgTrack.setFillColor(sf::Color(0, 0, 0), 0.2f);

        // 2. Setup Track Kích Hoạt (Fill)
        activeTrack.setSize({0.0f, trackHeight});
        activeTrack.setRadius(trackHeight / 2.0f);
        activeTrack.setPower(2.0f);
        activeTrack.setOrigin(0.0f, trackHeight / 2.0f);

        // Phần Fill thường là màu trắng hoặc xám sáng nổi bật
        activeTrack.setFillColor(sf::Color(255, 255, 255), 0.3f);
        activeTrack.setBakedGlass(&ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur.png"), sf::Vector2f(1920.f, 1080.f));

        // 4. Cấu hình độ cứng và ma sát của Lò xo (Spring Physics)
        thumbSpring.stiffness = 700.0f;
        thumbSpring.damping = 35.0f;
        thumbSpring.snapTo(currentValue);
    }

    void Slider::setPosition(sf::Vector2f pos)
    {
        position = pos;
        // Cập nhật vị trí của toàn bộ thành phần con
        bgTrack.setPosition(position);
        activeTrack.setPosition(position);

        // Tính toán lại vị trí icon dựa trên position gốc
        leftIcon.setPosition(position.x - 40.0f, position.y);
        rightIcon.setPosition(position.x + trackWidth + 40.0f, position.y);
    }

    void Slider::setRange(float minVal, float maxVal, float step)
    {
        minValue = minVal;
        maxValue = maxVal;
        stepSize = step;

        // Đảm bảo giá trị hiện tại không vượt quá giới hạn mới
        setValue(std::clamp(currentValue, minValue, maxValue));
    }

    void Slider::setValue(float val, bool instant)
    {
        currentValue = std::clamp(val, minValue, maxValue);

        // Nếu kéo thả (Drag) thì nhảy tức thì, nếu Click thì trượt (Slide)
        if (instant)
        {
            thumbSpring.snapTo(currentValue);
        }
        else
        {
            thumbSpring.target = currentValue;
        }

        float snappedValue = snap(currentValue);
        if (snappedValue != lastSentValue)
        {
            lastSentValue = snappedValue;
            if (onValueChanged)
            {
                onValueChanged(snappedValue);
            }
        }
    }

    void Slider::setOpacity(float opacity)
    {
        // Clamp giá trị opacity vào khoảng [0, 255] rồi chia lấy tỷ lệ
        opacityFactor = std::clamp(opacity, 0.0f, 255.0f) / 255.0f;
    }

    void Slider::setIcons(const sf::Texture& leftTex, const sf::Texture& rightTex)
    {
        leftIcon.setTexture(leftTex);
        rightIcon.setTexture(rightTex);

        sf::FloatRect lBounds = leftIcon.getLocalBounds();
        leftIcon.setOrigin(lBounds.width / 2.0f, lBounds.height / 2.0f);

        sf::FloatRect rBounds = rightIcon.getLocalBounds();
        rightIcon.setOrigin(rBounds.width / 2.0f, rBounds.height / 2.0f);

        leftIcon.setPosition(position.x - 35.0f, position.y);
        rightIcon.setPosition(position.x + trackWidth + 35.0f, position.y);

        leftIcon.setScale(25.f / lBounds.width, 25.f / lBounds.height);
        rightIcon.setScale(25.f / rBounds.width, 25.f / rBounds.height);
    }

    float Slider::snap(float value)
    {
        // Thuật toán bắt dính: Làm tròn giá trị theo bước nhảy (stepSize)
        if (stepSize <= 0.0f) return std::clamp(value, minValue, maxValue);

        float snapped = std::round(value / stepSize) * stepSize;
        return std::clamp(snapped, minValue, maxValue);
    }

    void Slider::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        // Bỏ qua tương tác nếu component đang tàng hình (opacity quá thấp)
        if (opacityFactor < 0.1f) return;

        sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixelPos);

        // Chuyển tọa độ chuột toàn cục sang tọa độ cục bộ của Slider
        sf::Vector2f localMousePos = mousePos - position;

        // Hitbox giờ bao trọn cái viên thuốc này
        sf::FloatRect hitbox(0.0f, -trackHeight / 2.0f, trackWidth, trackHeight);

        // 1. Hover Logic (Trạng thái trỏ chuột)
        if (event.type == sf::Event::MouseMoved)
        {
            isHovered = hitbox.contains(localMousePos);

            if (isDragging)
            {
                // Nếu đang kéo, tính toán tỷ lệ vị trí X của chuột so với track Width
                float ratio = localMousePos.x / trackWidth;
                ratio = std::clamp(ratio, 0.0f, 1.0f);

                float rawValue = minValue + ratio * (maxValue - minValue);

                if (currentValue != rawValue)
                {
                    setValue(rawValue, true); // Đồng bộ 1:1 với tọa độ chuột
                }
            }
        }

        // 2. Click Logic (Nhấn chuột)
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (isHovered)
            {
                isDragging = true;
                float ratio = localMousePos.x / trackWidth;
                ratio = std::clamp(ratio, 0.0f, 1.0f);
                float rawValue = minValue + ratio * (maxValue - minValue);

                setValue(rawValue, false); // Click tới đâu, thumb nhảy chính xác tới đó
            }
        }

        // 3. Release Logic (Nhả chuột)
        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            isDragging = false;
        }
    }

    void Slider::update(const sf::RenderWindow& window, float dt)
    {
        // Nếu không có event MouseMoved (chuột đứng im nhưng object trượt qua),
        // ta vẫn cần kiểm tra lại trạng thái Hover.
        if (!isDragging && opacityFactor > 0.1f)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f localMousePos = mousePos - position;
            // Hitbox giờ bao trọn cái viên thuốc này
            sf::FloatRect hitbox(0.0f, -trackHeight / 2.0f, trackWidth, trackHeight);

            bool currentHover = hitbox.contains(localMousePos);
            if (isHovered != currentHover)
            {
                isHovered = currentHover;
            }
        }

        // 1. Cập nhật hệ thống Vật lý Lò xo
        thumbSpring.update(dt);

        // 2. Tính toán chiều rộng hiện tại của phần Fill dựa trên giá trị của lò xo
        float currentSpringVal = std::clamp(thumbSpring.position, minValue, maxValue);
        float ratio = (currentSpringVal - minValue) / (maxValue - minValue);
        float currentWidth = trackHeight + ratio * (trackWidth - trackHeight);

        // 3. Cập nhật đồ họa
        // Cập nhật chiều rộng cho activeTrack
        activeTrack.setSize({currentWidth, trackHeight});

        // 4. Cập nhật Opacity (Morphing Effect)
        // Giả sử màu gốc là Đen cho nền, Trắng cho fill
        auto applyAlpha = [&](sf::Color color) -> sf::Color
        {
            color.a = static_cast<sf::Uint8>(color.a * opacityFactor);
            return color;
        };

        bgTrack.setFillColor(sf::Color(0, 0, 0), 0.2f * opacityFactor);
        activeTrack.setFillColor(sf::Color(255, 255, 255), 0.3f * opacityFactor);
        // 3. THÊM VIỀN: Tạo độ sắc sảo cho khối kính (Rất quan trọng)
        activeTrack.setOutlineColor(sf::Color(255, 255, 255, (sf::Uint8)(150 * opacityFactor)));
        activeTrack.setOutlineThickness(0.5f);

        leftIcon.setColor(applyAlpha(sf::Color(150, 150, 150)));
        rightIcon.setColor(applyAlpha(sf::Color(150, 150, 150)));
    }

    void Slider::draw(sf::RenderTarget& target)
    {
        if (opacityFactor <= 0.01f) return;

        target.draw(bgTrack);
        // Chỉ vẽ thanh xanh nếu độ dài của nó lớn hơn 0.1px
        if (activeTrack.getSize().x > 0.1f)
        {
            target.draw(activeTrack);
        }
//        window.draw(thumb);

        target.draw(leftIcon);
        target.draw(rightIcon);
    }
}
