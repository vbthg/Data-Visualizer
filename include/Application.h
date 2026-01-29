#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "State.h"
#include "TitleBar.h"

class Application
{
public:
    Application();
    ~Application();

    void run();

    // Các hàm quản lý State an toàn
    void pushState(State* state);
    void popState();
    void changeState(State* state); // Thay thế state hiện tại (Intro -> Menu)

private:
    sf::RenderWindow window;
    GUI::TitleBar* titleBar; // Để con trỏ để quản lý khởi tạo linh hoạt

    // Stack chứa các màn hình
    // Top của stack chính là màn hình đang hiện
    std::stack<State*> states;

    void handleGlobalInput(sf::Event& event);
};
