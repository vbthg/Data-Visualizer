#include "InputField.h"
#include "Theme.h"
#include <cctype> // Để dùng std::isdigit

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    InputField::InputField(const sf::Font& font, Type type, sf::Vector2f sizeParams)
        : inputType(type), size(sizeParams), focused(false), cursorTimer(0.0f), showCursor(false)
    {
        // 1. Setup Background (Squircle)
        bgShape.setSize(size);
        bgShape.setRadius(12.0f);
        bgShape.setOrigin(size.x / 2.0f, size.y / 2.0f);
        bgShape.setFillColor(sf::Color(245, 245, 245)); // Xám rất nhạt
        bgShape.setOutlineThickness(1.5f);
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
            bgShape.setOutlineColor(Theme::Color::Primary); // Bật viền xanh
            bgShape.setFillColor(sf::Color::White);
        }
        else
        {
            bgShape.setOutlineColor(sf::Color::Transparent); // Tắt viền
            bgShape.setFillColor(sf::Color(245, 245, 245));
        }
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
            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // Do Shape origin ở giữa, tính bounds bằng Transform
            sf::FloatRect hitbox(position.x - size.x/2.0f, position.y - size.y/2.0f, size.x, size.y);

            setFocus(hitbox.contains(mPos));
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
        if (focused) bgShape.setOutlineColor(Theme::Color::Primary);
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

    void InputField::draw(sf::RenderWindow& window)
    {
        window.draw(bgShape);

        if (rawString.empty() && !focused)
        {
            window.draw(placeholderText);
        }
        else
        {
            window.draw(displayText);

            // Chỉ vẽ cursor khi đang focus và đến nhịp nhấp nháy
            if (focused && showCursor)
            {
                window.draw(cursor);
            }
        }
    }
}
