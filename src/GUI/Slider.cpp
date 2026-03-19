#include "Slider.h"

namespace GUI
{
    Slider::Slider(float trackWidth)
        : trackWidth(trackWidth), minValue(0.f), maxValue(1.f), stepSize(0.1f),
          currentValue(0.5f), isDragging(false), isHovered(false), opacityFactor(1.0f), position(0.f, 0.f)
    {
        // 1. Setup Track Nền (Background Track)
        bgTrack.setSize({trackWidth, trackHeight});
        bgTrack.setCornerRadius(trackHeight / 2.0f, trackHeight / 2.0f, trackHeight / 2.0f, trackHeight / 2.0f);
        // Đặt Origin ở GIỮA mép trái để dễ dàng định vị theo chiều dọc
        bgTrack.setOrigin(0.0f, trackHeight / 2.0f);
        bgTrack.setFillColor(sf::Color(220, 220, 225)); // Màu xám nhạt

        // 2. Setup Track Kích Hoạt (Active Track - iOS Blue)
        activeTrack.setSize({0.0f, trackHeight});
        activeTrack.setCornerRadius(trackHeight / 2.0f, trackHeight / 2.0f, trackHeight / 2.0f, trackHeight / 2.0f);
        activeTrack.setOrigin(0.0f, trackHeight / 2.0f);
        activeTrack.setFillColor(sf::Color(0, 122, 255)); // Màu xanh nước biển chuẩn Apple

        // 3. Setup Cục lăn (Thumb)
        thumb.setRadius(thumbBaseRadius);
        // Đặt Origin vào đúng TÂM hình tròn để khi Scale lên (phóng to), nó phình đều ra 4 phía
        thumb.setOrigin(thumbBaseRadius, thumbBaseRadius);
        thumb.setFillColor(sf::Color::White);
        thumb.setOutlineThickness(1.0f);
        thumb.setOutlineColor(sf::Color(200, 200, 200, 150)); // Thêm viền xám mờ để tách biệt với nền trắng

        // 4. Cấu hình độ cứng và ma sát của Lò xo (Spring Physics)
        thumbSpring.stiffness = 700.0f;
        thumbSpring.damping = 35.0f;
        thumbSpring.snapTo(currentValue);

        scaleSpring.stiffness = 600.0f;
        scaleSpring.damping = 35.0f;
        scaleSpring.snapTo(1.0f);
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

        // Vùng Hitbox mở rộng (Fitts's Law): Thay vì chỉ bắt chính xác vào thanh cao 6px,
        // ta mở rộng vùng bắt chuột ra cao 40px (Y: -20 đến 20) và dài ra 2 đầu thêm 10px.
        sf::FloatRect hitbox(-10.0f, -20.0f, trackWidth + 20.0f, 40.0f);

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

        // 4. Xử lý hoạt ảnh phóng to cục Thumb khi Hover hoặc Dragging
        if (isDragging || isHovered)
        {
            scaleSpring.target = 1.25f; // Phóng to 30%
        }
        else
        {
            scaleSpring.target = 1.0f; // Trả về bình thường
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
            sf::FloatRect hitbox(-10.0f, -20.0f, trackWidth + 20.0f, 40.0f);

            bool currentHover = hitbox.contains(localMousePos);
            if (isHovered != currentHover)
            {
                isHovered = currentHover;
                scaleSpring.target = (isHovered) ? 1.3f : 1.0f;
            }
        }

        // 1. Cập nhật hệ thống Vật lý Lò xo
        thumbSpring.update(dt);
        scaleSpring.update(dt);

        // 2. Tính toán vị trí X hiện tại của cục lăn dựa trên giá trị của lò xo
        float currentSpringVal = std::clamp(thumbSpring.position, minValue, maxValue);
        float ratio = (currentSpringVal - minValue) / (maxValue - minValue);
        float currentX = ratio * trackWidth;

        // 3. Cập nhật đồ họa
        // Di chuyển cục thumb
        thumb.setPosition(position.x + currentX, position.y);
        // Áp dụng scale-up cho thumb
        thumb.setScale(scaleSpring.position, scaleSpring.position);
        // Thanh xanh (activeTrack) sẽ kéo dài từ 0 đến chính giữa cục thumb
        activeTrack.setSize({currentX, trackHeight});

        // 4. Cập nhật Opacity (Morphing Effect)
        auto applyAlpha = [&](sf::Color color) -> sf::Color
        {
            color.a = static_cast<sf::Uint8>(color.a * opacityFactor);
            return color;
        };

        bgTrack.setFillColor(applyAlpha(sf::Color(220, 220, 225)));
        activeTrack.setFillColor(applyAlpha(sf::Color(0, 122, 255)));
        thumb.setFillColor(applyAlpha(sf::Color::White));

        // Cập nhật cả Alpha của viền mờ xung quanh thumb
        sf::Color outlineCol = sf::Color(200, 200, 200, 150);
        thumb.setOutlineColor(applyAlpha(outlineCol));

        leftIcon.setColor(applyAlpha(sf::Color(150, 150, 150)));
        rightIcon.setColor(applyAlpha(sf::Color(150, 150, 150)));
    }

    void Slider::draw(sf::RenderWindow& window)
    {
        if (opacityFactor <= 0.01f) return;

        window.draw(bgTrack);
        // Chỉ vẽ thanh xanh nếu độ dài của nó lớn hơn 0.1px
        if (activeTrack.getSize().x > 0.1f)
        {
            window.draw(activeTrack);
        }
        window.draw(thumb);

        window.draw(leftIcon);
        window.draw(rightIcon);
    }
}
