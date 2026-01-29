#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include "State.h"
#include "MenuCard.h"
#include "Button.h"
#include "Squircle.h"         // <--- Thay RoundedRect bằng Squircle
#include "Theme.h" // <--- Include Theme

class MenuState : public State
{
public:
    MenuState(sf::RenderWindow& window, std::stack<State*>& states);
    virtual ~MenuState();

    void init();
    void handleInput(sf::Event& event);
    void update(float dt);
    void draw();

private:
    sf::RenderWindow& window;
    std::stack<State*>& states;

    // Layout
    sf::Text titleText;
    sf::Text subTitleText;
    std::vector<GUI::MenuCard*> cards;

    // State Management
    int selectedIndex;
    GUI::MenuCard* expandedCard;

    // Entry Animation
    float entryTimer;

    // Helper layout
    void updateLayout(float dt);

    // Màu nền động
    sf::Color currentWindowColor;

    // Helper
    sf::Color getPaleColor(sf::Color c);
};
