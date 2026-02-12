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
    void handleInput(sf::Event& event) override; // Override từ State class
    void update(float dt) override;
    void draw() override;

private:
    sf::RenderWindow& window;
    std::stack<State*>& states;

    // --- UI ELEMENTS ---
    sf::Text titleText;
    sf::Text subTitleText;

    GUI::Squircle bgBoard;

    // Danh sách 3 thẻ Categories
    std::vector<GUI::MenuCard*> cards;

    // --- STATE DATA (NORMAL) ---
    int selectedIndex;          // Index thẻ đang hover
    sf::Color currentWindowColor;

    // --- TRANSITION DATA (NEW - Dùng cho hiệu ứng chuyển cảnh) ---
    bool isTransitioning;       // Cờ chặn input khi đang bay
    float transitionTimer;      // Bộ đếm thời gian
    float transitionDuration;   // Tổng thời gian hiệu ứng

    GUI::MenuCard* expandedCard; // Con trỏ đến thẻ "Hero" (thẻ được chọn)

    // "Diễn viên đóng thế" cho hiệu ứng chữ bay (Text Morph)
    sf::Text transitionText;
    sf::Vector2f transitionStartPos;
    // [THÊM DÒNG NÀY] Biến lưu đích đến của chữ khi bay
    sf::Vector2f textTargetPos;

    // --- HELPER FUNCTIONS ---
    void updateLayout(float dt);         // Update cho trạng thái tĩnh (Accordion)
    void updateOutTransition(float dt);  // Update cho trạng thái động (Transition)

    // Helper cũ (giữ lại nếu bạn vẫn dùng đổi màu nền)
    sf::Color getPaleColor(sf::Color c);


    // [THÊM] Biến cờ để đánh dấu đã đẩy state mới hay chưa
    bool hasPushedState = false;
};
