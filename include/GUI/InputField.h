#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Squircle.h"
#include "Spring.h"

namespace GUI
{
    class InputField : public sf::Drawable, public sf::Transformable
    {
    public:
        enum class Type
        {
            Integer, // Chỉ nhận số, chặn tràn giới hạn int
            String   // Nhận mọi ký tự
        };

        InputField(const sf::Font& font, Type type, sf::Vector2f size = {200.f, 44.f});

        // --- Cấu hình ---
        void setPosition(sf::Vector2f pos);
        sf::Vector2f getPosition() const;
        sf::Vector2f getSize() const;

        void setPlaceholder(const std::string& text);
        std::string getText() const;
        void clear();

        // --- Core ---
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void update(float dt);
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states)

        bool isFocused() const { return focused; }
        void setFocus(bool focus);

        // Thêm hàm này để đồng bộ Glassmorphism nếu cần
        void setBackgroundTexture(const sf::Texture* tex, sf::Vector2f res = {1920.f, 1080.f})
        {
            bgShape.setBakedGlass(tex, res);
        }

    private:
        Type inputType;

        // Layout & Visuals
        sf::Vector2f position;
        sf::Vector2f size;
        Squircle bgShape;

        sf::Text displayText;
        sf::Text placeholderText;
        sf::RectangleShape cursor;

        std::string rawString;

        // Trạng thái
        bool focused;
        float cursorTimer;
        bool showCursor;

        // Hiệu ứng Lắc (Shake)
        Utils::Physics::Spring shakeSpring;

        // Xử lý logic nội bộ
        void triggerErrorShake();
        bool isValidInput(sf::Uint32 unicode);
        bool wouldOverflowInt(const std::string& newStr);
        void updateTextLayout();
    };
}
