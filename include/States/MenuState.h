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

    GUI::Squircle bgBoard;

    int currentCategory; // Lưu ID category đang chọn

    bool isAnimating; // Cờ báo hiệu có thẻ đang di chuyển

    // Layout
    sf::Text titleText;
    sf::Text subTitleText;
    std::vector<GUI::MenuCard*> cards;

    // State Management
    int selectedIndex;
    GUI::MenuCard* expandedCard;

    // Nút Back để quay về CategoriesState
    GUI::Button* btnBack;
    sf::Text backIcon;
    sf::Font fontIcon;

    // Entry Animation
    float entryTimer;

    bool isTransitioning = true; // Mặc định vào là chạy hiệu ứng ngay
    float transitionTimer = 0.0f;

    // Helper layout
    void updateLayout(float dt);

    // Màu nền động
    sf::Color currentWindowColor;

    // Helper
    sf::Color getPaleColor(sf::Color c);

    // Dữ liệu
    std::vector<CategoryInfo> allData;
    void initData();

    // Hàm update riêng cho hiệu ứng
    void updateInTransition(float dt);
};
