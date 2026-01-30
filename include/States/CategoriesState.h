#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include "State.h"
#include "GUI/MenuCard.h"
#include "GUI/Button.h"
#include "GUI/Squircle.h"
#include "Utils/Graphics/Theme.h"

class CategoriesState : public State
{
public:
    CategoriesState(sf::RenderWindow& window, std::stack<State*>& states);
    virtual ~CategoriesState();

    void init();
    void handleInput(sf::Event& event);
    void update(float dt);
    void draw();

private:
    sf::RenderWindow& window;
    std::stack<State*>& states;

    // --- UI ELEMENTS ---
    sf::Text titleText;
    sf::Text subTitleText;

    // Danh sách 3 thẻ Categories
    std::vector<GUI::MenuCard*> cards;

    // --- STATE DATA ---
    int selectedIndex;          // Index thẻ đang hover (để làm hiệu ứng accordion)

    // --- HELPER FUNCTIONS ---
    void updateLayout(float dt);

    // Màu nền (nếu muốn đổi màu background khi hover)
    sf::Color currentWindowColor;
    sf::Color getPaleColor(sf::Color c);
};
