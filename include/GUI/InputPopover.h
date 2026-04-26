#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Squircle.h"
#include "Button.h"
#include "InputField.h"
#include "Spring.h"
#include "Command.h"

namespace GUI
{
    class InputPopover
    {
    public:
        enum class State
        {
            Closed,
            Opening,
            Open,
            Closing
        };

        InputPopover(const sf::Font& font);
        ~InputPopover();

        // Kích hoạt bảng Morphing
        // startPos: Tọa độ tâm của nút trên Dock
        void open(sf::Vector2f startPos, const DS::Command& cmd);
        void close();

        void setInputType(DS::InputType inputType);

        bool handleEvent(const sf::Event& event, sf::RenderWindow& window);
        void update(float dt, sf::RenderWindow& window);
        void draw(sf::RenderWindow& window);

        bool isOpen() const { return currentState != State::Closed; }

    private:
        State currentState;
        DS::Command currentCommand;

        // Lưu lại vị trí nút để lúc đóng bay về đúng chỗ đó
        sf::Vector2f originPos;

        // --- Visual Components ---
        Squircle background;
        sf::Text titleText;

        // Tối đa 2 ô nhập liệu
        InputField* input1;
        InputField* input2;

        Button* btnConfirm;
        Button* btnCancel;

        // --- Physics (Apple Spring) ---
        Utils::Physics::Spring xSpring;
        Utils::Physics::Spring ySpring;
        Utils::Physics::Spring wSpring;
        Utils::Physics::Spring hSpring;
        Utils::Physics::Spring rSpring; // Bo góc
        Utils::Physics::Spring alphaSpring; // Độ mờ của nội dung bên trong

        void submitData();
        void updateLayout(); // Căn chỉnh các thành phần bên trong khi Morphing
    };
}
