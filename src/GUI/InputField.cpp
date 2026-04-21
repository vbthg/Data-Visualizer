#include "InputField.h"
#include "ViewHandler.h"
#include "Theme.h"
#include <cctype> // Để dùng std::isdigit

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    InputField::InputField(const sf::Font& font, Type type, sf::Vector2f sizeParams)
        : inputType(type), size(sizeParams), focused(false), cursorTimer(0.0f), showCursor(false)
    {
        // 1. Setup Background (Squircle)
        bgShape.setPower(2.f);
        bgShape.setSize(size);
        bgShape.setRadius(size.x / 2.f);
        bgShape.setOrigin(size.x / 2.0f, size.y / 2.0f);
        bgShape.setFillColor(sf::Color(245, 245, 245)); // Xám rất nhạt
        bgShape.setOutlineThickness(1.f);
        bgShape.setOutlineColor(sf::Color::Transparent);

        // 2. Setup Texts
        displayText.setFont(font);
        displayText.setCharacterSize(18);
        displayText.setFillColor(Theme::Color::TextPrimary);

        placeholderText.setFont(font);
        placeholderText.setCharacterSize(18);
        placeholderText.setFillColor(sf::Color(150, 150, 150));

        // 3. Setup Cursor (Con trỏ nhấp nháy)
        cursor.setSize({2.0f, 20.0f});
        cursor.setFillColor(Theme::Color::Primary);
        cursor.setOrigin(0.0f, 10.0f);

        // 4. Setup Physics cho hiệu ứng Lắc
        // Cần độ cứng cực cao và ma sát thấp để nảy qua lại gắt
        shakeSpring.stiffness = 800.0f;
        shakeSpring.damping = 15.0f;
        shakeSpring.target = 0.0f; // Luôn hướng về vị trí 0
    }

    void InputField::setPosition(sf::Vector2f pos)
    {
        position = pos;
        // Cập nhật tọa độ sẽ được tính toán lại trong hàm update (cộng thêm shake)
    }

    sf::Vector2f InputField::getPosition() const { return position; }
    sf::Vector2f InputField::getSize() const { return size; }
    std::string InputField::getText() const { return rawString; }

    void InputField::clear()
    {
        rawString = "";
        updateTextLayout();
    }

    void InputField::setPlaceholder(const std::string& text)
    {
        placeholderText.setString(text);

        // Căn giữa Placeholder
        sf::FloatRect bounds = placeholderText.getLocalBounds();
        placeholderText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }

    void InputField::setFocus(bool focus)
    {
        focused = focus;
        cursorTimer = 0.0f;
        showCursor = true;

        if (focused)
        {
            // Màu Apple Focus: Trắng mờ hơn một chút để thấy chiều sâu
            bgShape.setFillColor(sf::Color(255, 255, 255, 40));
            bgShape.setOutlineColor(sf::Color(245, 245, 247, 180));
        }
        else
        {
            // Màu Apple Idle: Trắng cực mờ
            bgShape.setFillColor(sf::Color(255, 255, 255, 20));
            bgShape.setOutlineColor(sf::Color::Transparent);
        }
    }

 void InputField::setGlobalAlpha(float alpha)
{
    float a = std::max(0.0f, std::min(1.0f, alpha));
    sf::Uint8 alphaByte = static_cast<sf::Uint8>(a * 255.f);

    // 1. Màu nền: Tăng Alpha lên để nó nổi bật trên nền đen
    sf::Color fillCol = sf::Color::White;

    // Tăng từ 45 lên 80 khi focus, và từ 25 lên 50 khi idle
    sf::Uint8 targetAlpha = focused ? 40 : 20;
    fillCol.a = static_cast<sf::Uint8>(a * targetAlpha);
    bgShape.setFillColor(fillCol);

    // 2. Màu viền: Cho nó sáng hơn một chút
//    sf::Color outlineCol = focused ? Theme::Color::Primary : sf::Color(255, 255, 255, 30);
//    sf::Color outlineCol = focused ? sf::Color(245, 245, 247, 180) : sf::Color(255, 255, 255, 100);
    sf::Color outlineCol = sf::Color(245, 245, 247, 180);
    // Nếu đang focus thì hiện rõ, không focus thì hiện viền trắng siêu mờ
    sf::Uint8 outlineAlpha = focused ? 180 : 0;
    outlineCol.a = static_cast<sf::Uint8>(a * outlineAlpha);
//    outlineCol.a = static_cast<sf::Uint8>(180);
    bgShape.setOutlineColor(outlineCol);

    // 3. Chữ và Placeholder: Phải là TRẮNG TUYỀN (255, 255, 255)
    // Đừng dùng Theme::Color::TextPrimary nếu nó là màu xám/đen
    displayText.setFillColor(sf::Color(255, 255, 255, alphaByte));

    sf::Color phCol = sf::Color(255, 255, 255, static_cast<sf::Uint8>(alphaByte * 0.5f));
    placeholderText.setFillColor(phCol);

    // 4. Con trỏ
    cursor.setFillColor(sf::Color(Theme::Color::Primary.r,
                                  Theme::Color::Primary.g,
                                  Theme::Color::Primary.b, alphaByte));
}

    void InputField::setSize(sf::Vector2f size)
    {
        size = size;
        bgShape.setSize(size);

        // Mỗi lần đổi size, text cần được tính lại vị trí để luôn ở giữa chiều cao
        updateTextPosition();
    }

    void InputField::updateTextPosition()
    {
        // Căn văn bản theo trục dọc (Vertical centering)
        // Giả sử có 15px lề trái (Padding)
        float paddingLeft = 15.f;
        float textY = size.y / 2.0f;

        displayText.setOrigin(0.f, displayText.getGlobalBounds().height / 2.0f);
        displayText.setPosition(paddingLeft, textY);

        // Tương tự cho placeholder
        placeholderText.setOrigin(0.f, placeholderText.getGlobalBounds().height / 2.0f);
        placeholderText.setPosition(paddingLeft, textY);
    }

    void InputField::triggerErrorShake()
    {
        // Kéo lò xo lệch sang phải 15px. Nó sẽ tự động bật ngược lại qua trái do target = 0
        shakeSpring.position = 15.0f;
        shakeSpring.velocity = 0.0f; // Reset gia tốc

        // Chớp viền đỏ báo lỗi
        bgShape.setOutlineColor(Theme::Color::Danger);
    }

    bool InputField::isValidInput(sf::Uint32 unicode)
    {
        if (inputType == Type::Integer)
        {
            // Cho phép dấu âm ở đầu
            if (unicode == '-' && rawString.empty()) return true;
            // Chỉ cho phép số
            if (unicode >= '0' && unicode <= '9') return true;

            return false;
        }
        return true; // String cho phép mọi ký tự in được
    }

    bool InputField::wouldOverflowInt(const std::string& newStr)
    {
        if (newStr.empty() || newStr == "-") return false;

        // Giới hạn độ dài tuyệt đối để tránh crash std::stoll
        if (newStr.length() > 11) return true;

        try
        {
            long long val = std::stoll(newStr);
            if (val > 2147483647LL || val < -2147483648LL)
                return true;
        }
        catch (...)
        {
            return true; // Lỗi chuyển đổi = quá lớn
        }
        return false;
    }

    void InputField::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        // 1. Xử lý Click chuột để Focus
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
//            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2i mPos = {event.mouseButton.x, event.mouseButton.y};

            // Do Shape origin ở giữa, tính bounds bằng Transform
            sf::FloatRect hitbox(position.x - size.x / 2.0f, position.y - size.y / 2.0f, size.x, size.y);

//            setFocus(hitbox.contains(mPos));
            setFocus(Utils::ViewHandler::isMouseInFrame(mPos, window, hitbox));
        }

        // 2. Xử lý Gõ phím
        if (focused && event.type == sf::Event::TextEntered)
        {
            // Phím Backspace (Mã ASCII 8)
            if (event.text.unicode == 8)
            {
                if (!rawString.empty())
                {
                    rawString.pop_back();
                    updateTextLayout();
                }
            }
            // Các ký tự in được (Mã ASCII > 31 và không phải Delete 127)
            else if (event.text.unicode > 31 && event.text.unicode < 127)
            {
                // Kiểm tra ký tự có hợp lệ không (VD: nhập chữ vào ô số)
                if (!isValidInput(event.text.unicode))
                {
                    triggerErrorShake();
                    return;
                }

                char addedChar = static_cast<char>(event.text.unicode);
                std::string testString = rawString + addedChar;

                // Kiểm tra giới hạn Int
                if (inputType == Type::Integer && wouldOverflowInt(testString))
                {
                    triggerErrorShake();
                    return;
                }

                // Kiểm tra giới hạn String (Tối đa 25 ký tự cho đẹp giao diện)
                if (inputType == Type::String && testString.length() > 25)
                {
                    triggerErrorShake();
                    return;
                }

                // Hợp lệ -> Thêm vào chuỗi
                rawString += addedChar;
                updateTextLayout();
            }
        }
    }

    void InputField::updateTextLayout()
    {
        displayText.setString(rawString);
        // Reset thời gian nháy con trỏ mỗi khi gõ
        cursorTimer = 0.0f;
        showCursor = true;

        // Nếu focus bị mất viền đỏ (do vừa shake), trả lại màu xanh
//        if (focused) bgShape.setOutlineColor(Theme::Color::Primary);
        if (focused) bgShape.setOutlineColor(sf::Color(245, 245, 247, 180));
    }

    void InputField::update(float dt)
    {
        // 1. Nhấp nháy con trỏ
        if (focused)
        {
            cursorTimer += dt;
            if (cursorTimer >= 0.5f) // Nháy mỗi nửa giây
            {
                showCursor = !showCursor;
                cursorTimer = 0.0f;
            }
        }

        // 2. Cập nhật Vật lý Lắc (Shake)
        shakeSpring.update(dt);

        // Áp dụng tọa độ (Cộng thêm độ lệch của lò xo vào vị trí X)
        float currentX = position.x + shakeSpring.position;
        float currentY = position.y;

        bgShape.setPosition(currentX, currentY);
        placeholderText.setPosition(currentX, currentY);

        // 3. Căn chỉnh Text (Luôn cách lề trái 15px)
        float paddingLeft = 15.0f;
        float textStartX = currentX - size.x / 2.0f + paddingLeft;

        // LƯU Ý: SFML render text hơi lệch y, cần tự bù trừ tâm
        displayText.setPosition(textStartX, currentY - 12.0f);

        // 4. Di chuyển Cursor theo sau Text
        float textWidth = displayText.getLocalBounds().width;
        // Nếu chuỗi rỗng thì cursor đứng ở đầu
        float cursorX = rawString.empty() ? textStartX : textStartX + textWidth + 2.0f;

        cursor.setPosition(cursorX, currentY);
    }

    void InputField::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Kết hợp transform của Notch và transform nội bộ của InputField
        states.transform *= getTransform();

        target.draw(bgShape, states);
        target.draw(displayText, states);
        if (rawString.empty() && !focused) target.draw(placeholderText, states);
        if (focused && showCursor) target.draw(cursor, states);
    }

//    void InputField::draw(sf::RenderWindow& window)
//    {
//        window.draw(bgShape);
//
//        if (rawString.empty() && !focused)
//        {
//            window.draw(placeholderText);
//        }
//        else
//        {
//            window.draw(displayText);
//
//            // Chỉ vẽ cursor khi đang focus và đến nhịp nhấp nháy
//            if (focused && showCursor)
//            {
//                window.draw(cursor);
//            }
//        }
//    }
}
