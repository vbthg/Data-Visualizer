#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include "State.h"
#include "MenuCard.h"
#include "Button.h"
#include "Squircle.h"         // <--- Thay RoundedRect bằng Squircle
#include "Theme.h" // <--- Include Theme

// Struct dữ liệu (định nghĩa ngay tại đây hoặc trong 1 file common)
struct AlgoInfo { int id; std::string name; std::string number; };
struct CategoryInfo { std::string name; sf::Color color; std::vector<AlgoInfo> algos; };

class MenuState : public State
{
public:
    MenuState(sf::RenderWindow& window, std::stack<State*>& states, int categoryId);
    virtual ~MenuState();

    void init();
    void handleInput(sf::Event& event);
    void update(float dt);
    void draw();

private:
    sf::RenderWindow& window;
    std::stack<State*>& states;

    int currentCategory; // Lưu ID category đang chọn

    // Layout
    sf::Text titleText;
    sf::Text subTitleText;
    std::vector<GUI::MenuCard*> cards;

    // State Management
    int selectedIndex;
    GUI::MenuCard* expandedCard;

    // Nút Back để quay về CategoriesState
    GUI::Button* btnBack;

    // Entry Animation
    float entryTimer;

    // Helper layout
    void updateLayout(float dt);

    // Màu nền động
    sf::Color currentWindowColor;

    // Helper
    sf::Color getPaleColor(sf::Color c);

    // Dữ liệu
    std::vector<CategoryInfo> allData;
    void initData();
};
