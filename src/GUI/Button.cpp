#include "Button.h"
#include <iostream>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    Button::Button(const sf::Font& font, const sf::String& text, sf::Vector2f size)
        : size(size)
    {
        // 1. Setup Text
        content.setFont(font);
        content.setString(text);
        content.setCharacterSize(16);

        // 2. Setup Background (RoundedRect)
        bgShape.setSize(size);
        bgShape.setCornerRadius(Theme::Style::ButtonRadius, Theme::Style::ButtonRadius, Theme::Style::ButtonRadius, Theme::Style::ButtonRadius); // Mặc định bo 12px

        // QUAN TRỌNG: Đặt Origin vào TÂM để lò xo Scale hoạt động đúng từ giữa ra
        bgShape.setOrigin(size.x / 2.0f, size.y / 2.0f);

        // 3. Setup Physics (Apple Style: Cứng & Đầm)
        scaleSpring.stiffness = Theme::Animation::ButtonStiffness;
        scaleSpring.damping = Theme::Animation::ButtonDamping;
        scaleSpring.snapTo(1.0f);

        // 4. Init State
        position = {0, 0};
        isHovered = false;
        isPressed = false;

        currentBgColor = sf::Color(240, 240, 240);
        currentTextColor = sf::Color::Black;

        applyPreset(ButtonPreset::Secondary);

        // Cập nhật vị trí lần đầu
        setPosition(position);
    }

    // --- SETTERS ---

    void Button::setPosition(sf::Vector2f pos)
    {
        position = pos;
        // Vì Origin của bgShape đã ở tâm, nên setPosition sẽ đặt TÂM nút vào vị trí pos
        bgShape.setPosition(position);
        centerText();
    }

    void Button::setSize(sf::Vector2f s)
    {
        size = s;
        bgShape.setSize(size);
        // Cập nhật lại Origin vì size đổi
        bgShape.setOrigin(size.x / 2.0f, size.y / 2.0f);
        // Reset lại vị trí để đảm bảo khớp
        bgShape.setPosition(position);
        centerText();
    }

    void Button::setCornerRadius(float radius)
    {
        // Gọi thẳng hàm của RoundedRectangleShape
        bgShape.setCornerRadius(radius, radius, radius, radius);
    }

    void Button::setText(const sf::String& text)
    {
        content.setString(text);
        centerText();
    }

    void Button::setCharacterSize(unsigned int s)
    {
        content.setCharacterSize(s);
        centerText();
    }

    void Button::setFont(const sf::Font& font)
    {
        content.setFont(font);
        centerText();
    }

    void Button::setOutline(float thickness, sf::Color color)
    {
        // RoundedRectangleShape chưa hỗ trợ setOutline trực tiếp vì nó wrap ConvexShape
        // Nhưng ta có thể thêm hàm đó vào RoundedRectangleShape hoặc gọi:
        // Tuy nhiên, RoundedRectangleShape của bạn hiện tại chưa public ConvexShape.
        // -> BẠN NÊN THÊM hàm setOutline vào RoundedRectangleShape.h
        // Tạm thời comment nếu chưa update RoundedRect
        // bgShape.setOutlineThickness(thickness);
        // bgShape.setOutlineColor(color);
    }

    // --- COLOR CONFIG ---

    void Button::setBackgroundColor(sf::Color normal, sf::Color hover, sf::Color pressed)
    {
        bgColors = {normal, hover, pressed};
    }

    void Button::setTextColor(sf::Color normal, sf::Color hover, sf::Color pressed)
    {
        textColors = {normal, hover, pressed};
        content.setFillColor(normal);
    }

    void Button::setTextColor(sf::Color color)
    {
        textColors = {color, color, color};
        currentTextColor = color;
        content.setFillColor(currentTextColor);
    }

    void Button::applyPreset(ButtonPreset preset)
    {
        switch (preset)
        {
        case ButtonPreset::Primary: // Xanh Apple
            setBackgroundColor(sf::Color(0, 122, 255), sf::Color(0, 113, 227), sf::Color(0, 85, 170));
            setTextColor(sf::Color::White);
            break;

        case ButtonPreset::Secondary: // Xám
            setBackgroundColor(sf::Color(240, 240, 240), sf::Color(230, 230, 230), sf::Color(210, 210, 210));
            setTextColor(sf::Color::Black);
            break;

        case ButtonPreset::Ghost: // Viền mờ (Micro interaction)
            bgColors = {
                sf::Color(255, 255, 255, 0),   // Normal: Trong suốt
                sf::Color(255, 255, 255, 20),  // Hover: Trắng mờ 10%
                sf::Color(255, 255, 255, 40)   // Press: Trắng mờ 20%
            };
            // Giữ nguyên màu chữ hiện tại
            break;

        case ButtonPreset::Clean:
            setBackgroundColor(sf::Color::White, sf::Color(245, 245, 247), sf::Color(230, 230, 235));
            setTextColor(sf::Color::Black);
            break;

        case ButtonPreset::Danger:
            setBackgroundColor(sf::Color(255, 59, 48, 0), sf::Color(255, 59, 48, 30), sf::Color(255, 59, 48, 60));
            setTextColor(sf::Color(255, 59, 48));
            break;
        }
    }

    void Button::centerText()
    {
        sf::FloatRect bounds = content.getLocalBounds();
        // Đặt Origin chữ vào tâm của chính nó
        content.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        // Đặt vị trí chữ vào vị trí nút (vốn là tâm nút)
        content.setPosition(position);
    }

    // --- UPDATE & DRAW ---

    void Button::update(sf::RenderWindow& window, float dt)
    {
        // 1. Mouse Hit Test (Dùng hàm getGlobalBounds của RoundedRect)
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mPosF = window.mapPixelToCoords(mousePos);

        isHovered = bgShape.getGlobalBounds().contains(mPosF);

        // 2. Xác định màu mục tiêu
        sf::Color targetBg, targetText;

        if (isPressed)
        {
            targetBg = bgColors.pressed;
            targetText = textColors.pressed;
        }
        else if (isHovered)
        {
            targetBg = bgColors.hover;
            targetText = textColors.hover;
        }
        else
        {
            targetBg = bgColors.normal;
            targetText = textColors.normal;
        }

        // 3. Chuyển màu nhanh (Snappy) - minSpeed = 500
        currentBgColor = Utils::Math::Smoothing::dampColor(currentBgColor, targetBg, Theme::Animation::ColorSmoothing, dt, Theme::Animation::ColorSnapSpeed);
        currentTextColor = Utils::Math::Smoothing::dampColor(currentTextColor, targetText, Theme::Animation::ColorSmoothing, dt, Theme::Animation::ColorSnapSpeed);

        bgShape.setFillColor(currentBgColor);
        content.setFillColor(currentTextColor);

        // 4. Cập nhật Vật Lý Lò Xo
        scaleSpring.update(dt);
        float s = scaleSpring.position;

        // Apply Scale (RoundedRectangleShape đã hỗ trợ setScale)
        bgShape.setScale(s, s);
        content.setScale(s, s);
    }

    void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        // 1. Hover Logic (Micro Interaction: Không scale, chỉ đảm bảo target=1.0)
        if (bgShape.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))))
        {
            if (!isPressed) scaleSpring.target = 1.0f;
        }

        // 2. Click Logic
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (isHovered)
            {
                isPressed = true;
                scaleSpring.target = 0.96f; // Nhấn: Co nhẹ
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            if (isPressed)
            {
                isPressed = false;
                scaleSpring.target = 1.0f; // Thả: Bật về

                if (isHovered && onClick)
                {
                    onClick();
                }
            }
        }
    }

    void Button::draw(sf::RenderWindow& window)
    {
        window.draw(bgShape); // RoundedRectangleShape kế thừa sf::Drawable
        window.draw(content);
    }

} // namespace GUI
