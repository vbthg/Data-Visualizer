#include "Application.h"
#include "IntroState.h"
#include <iostream>

Application::Application()
{
    // 1. Setup Window
    window.create(sf::VideoMode::getDesktopMode(), "Algorithm Visualizer", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    // 2. Load Font qua ResourceManager (Singleton)
    // LƯU Ý: Xóa hết các dòng static sf::Font cũ đi
    sf::Font& iconFont = ResourceManager::getInstance().getFont("assets/fonts/Segoe MDL2 Assets.TTF");

    // 3. Khởi tạo TitleBar
    titleBar = new GUI::TitleBar(window, iconFont);

    // 4. Push IntroState (Không cần truyền resource manager nữa)
    pushState(new IntroState(window, states));
}

Application::~Application()
{
    // Dọn dẹp bộ nhớ
    while(!states.empty())
    {
        delete states.top();
        states.pop();
    }
    delete titleBar;
}

void Application::run()
{
    sf::Clock clock;

    while(window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        sf::Event event;

        // Nếu hết state thì đóng app
        if(states.empty())
        {
            window.close();
            break;
        }

        // Lấy state hiện tại (Màn hình trên cùng)
        State* currentState = states.top();

        // 1. INPUT
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed) window.close();

            // Xử lý Global (F11, Resize)
            handleGlobalInput(event);

            // Xử lý TitleBar (Kéo, Nút bấm)
            titleBar->handleEvent(event);

            // Xử lý input riêng của State (Intro, Menu...)
            currentState->handleInput(event);
        }

        // 2. UPDATE
        titleBar->update(dt);
        currentState->update(dt);

        // 3. DRAW
        window.clear(sf::Color::White);

        // Vẽ State nằm dưới (Background)
        currentState->draw();

        // TitleBar luôn vẽ đè lên trên cùng
        titleBar->draw();

        window.display();
    }
}

void Application::handleGlobalInput(sf::Event& event)
{
    // F11: Fullscreen Toggle
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
    {
        titleBar->toggleFullscreen();
    }

    // Fix méo hình khi resize
    if(event.type == sf::Event::Resized)
    {
        sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);
        window.setView(sf::View(visibleArea));
    }
}

// --- STATE MANAGEMENT ---

void Application::pushState(State* state)
{
    states.push(state);
}

void Application::popState()
{
    if(!states.empty())
    {
        delete states.top(); // Xóa vùng nhớ
        states.pop();        // Bỏ khỏi stack
    }
}

void Application::changeState(State* state)
{
    // Intro -> Menu: Ta muốn xóa Intro đi rồi mới thêm Menu
    popState();
    pushState(state);
}
