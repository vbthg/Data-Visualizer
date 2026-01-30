#include "MenuState.h"
#include "MenuCard.h"
#include "ResourceManager.h"
#include "Squircle.h"
#include "Smoothing.h"
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <algorithm>

namespace Theme = Utils::Graphics::Theme;

sf::Color MenuState::getPaleColor(sf::Color c)
{
    float factor = 0.4f;
    return sf::Color(
        (sf::Uint8)(c.r * factor + 255 * (1.0f - factor)),
        (sf::Uint8)(c.g * factor + 255 * (1.0f - factor)),
        (sf::Uint8)(c.b * factor + 255 * (1.0f - factor))
    );
}

MenuState::MenuState(sf::RenderWindow& window, std::stack<State*>& states, int categoryId)
    : window(window), states(states), currentCategory(categoryId)
{
    init();
}

MenuState::~MenuState()
{
    for(auto card : cards) delete card;
    cards.clear();
}

void MenuState::initData()
{
    allData.clear();

    // ID 0: Linear
    CategoryInfo cat1; cat1.name = "Linear Structures"; cat1.color = sf::Color(72, 219, 251);
    cat1.algos = {{0, "Singly Linked List", "01"}, {1, "Doubly Linked List", "02"}, {2, "Stack & Queue", "03"}, {3, "Circular List", "04"}};
    allData.push_back(cat1);

    // ID 1: Tree
    CategoryInfo cat2; cat2.name = "Tree Structures"; cat2.color = sf::Color(255, 159, 67);
    cat2.algos = {{4, "Binary Search Tree", "05"}, {5, "AVL Tree", "06"}, {6, "Binary Heap", "07"}, {7, "2-3-4 Tree", "08"}};
    allData.push_back(cat2);

    // ID 2: Advanced
    CategoryInfo cat3; cat3.name = "Advanced Algo"; cat3.color = sf::Color(95, 39, 205);
    cat3.algos = {{8, "Hash Table", "09"}, {9, "Graph (BFS/DFS)", "10"}, {10, "MST (Prim)", "11"}, {11, "Dijkstra", "12"}};
    allData.push_back(cat3);
}

void MenuState::init()
{
    // 1. Load Data
    initData();
    if (currentCategory < 0 || currentCategory >= (int)allData.size()) currentCategory = 0;
    CategoryInfo& info = allData[currentCategory];

    auto& res = ResourceManager::getInstance();
    sf::Font& fontHeader = res.getFont("assets/fonts/font.ttf");

    // --- SETUP HEADER (Theo tên Category) ---
    titleText.setFont(fontHeader);
    titleText.setString(info.name); // <--- Lấy tên từ dữ liệu
    titleText.setCharacterSize(34);
    titleText.setFillColor(Theme::Color::TextPrimary);

    subTitleText.setFont(res.getFont("assets/fonts/font.ttf"));
    subTitleText.setString("Select to visualize");
    subTitleText.setCharacterSize(18);
    subTitleText.setFillColor(Theme::Color::TextSecondary);

    // --- CREATE CARDS (Theo danh sách thuật toán) ---
    // Xóa thẻ cũ (nếu có)
    for(auto c : cards) delete c;
    cards.clear();

    // Tạo thẻ động
    for (const auto& algo : info.algos)
    {
        // Truyền ID, Tên, Số, Màu của Category hiện tại
        cards.push_back(new GUI::MenuCard(algo.id, algo.name, algo.number, info.color));
    }

    // --- INIT STATES ---
    selectedIndex = -1;
    expandedCard = nullptr;
    entryTimer = 0.0f;

    currentWindowColor = Theme::Color::Background;

    updateLayout(0.0f);
}

void MenuState::updateLayout(float dt)
{
    float winW = (float)window.getSize().x;
    float winH = (float)window.getSize().y;

    // --- 1. DÙNG HẰNG SỐ MỚI TỪ THEME ---
    float containerW = Theme::Style::AlgoBoardW; // 1200.0f
    float containerH = Theme::Style::AlgoBoardH; // 580.0f

    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;

    titleText.setPosition(startX, startY - 80.f);
    subTitleText.setPosition(startX, startY - 40.f);

    // --- 2. BACKGROUND COLOR LOGIC ---
    sf::Color targetWinColor = Theme::Color::Background;
    if(expandedCard) {
        targetWinColor = getPaleColor(expandedCard->getThemeColor());
    }
    currentWindowColor = Utils::Math::Smoothing::dampColor(currentWindowColor, targetWinColor, Theme::Animation::BgSmoothing, dt, Theme::Animation::BgMinSpeed);

    // --- 3. CARD LOGIC (HERO ANIMATION) ---
    if(expandedCard != nullptr)
    {
        // MODE: EXPANDED
        for(auto c : cards)
        {
            if(c == expandedCard)
            {
                // Thẻ Hero chiếm trọn bảng 1200x580
                c->setTarget({startX, startY}, {containerW, containerH});
                c->setExpanded(true);
            }
            else
            {
                // Các thẻ khác trượt ra ngoài
                float offX = (c->getId() < expandedCard->getId()) ? startX - 300.f : startX + containerW + 300.f;
                c->setTarget({offX, startY}, {containerW / 4.f, containerH});
                c->setExpanded(false);
            }
        }
    }
    else
    {
        // MODE: ACCORDION
        float w = containerW / cards.size(); // Chia đều theo số lượng thẻ thực tế
        float currentX = startX;

        // Dùng ItemRadius (48.0f) cho các thẻ con
        float cornerRadius = Theme::Style::ItemRadius;

        for(int i = 0; i < (int)cards.size(); ++i)
        {
            cards[i]->setTarget({currentX, startY}, {w, containerH});
            cards[i]->setSelected(i == selectedIndex);
            cards[i]->setExpanded(false);

            GUI::CardPos pos = GUI::CardPos::Middle;
            if(i == 0) pos = GUI::CardPos::First;
            else if(i == (int)cards.size() - 1) pos = GUI::CardPos::Last;

            cards[i]->setCardPosition(pos, cornerRadius);

            currentX += w;
        }
    }

    for(auto c : cards) c->update(dt, window);
}

void MenuState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Closed) window.close();

    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        if(expandedCard) expandedCard = nullptr;
        else {
             // states.pop(); // Bỏ comment nếu muốn ESC quay về Categories
             window.close();
        }
    }

    for(int i = 0; i < (int)cards.size(); ++i)
    {
        if(expandedCard && cards[i] != expandedCard) continue;

        // Callback Functions
        auto onSelect = [this, i]() {
            if(!this->expandedCard) {
                if(this->selectedIndex == i) this->selectedIndex = -1;
                else this->selectedIndex = i;
            }
        };

        auto onViewMore = [this, i]() {
            this->expandedCard = this->cards[i];
        };

        auto onStart = [this, i]() {
            // Lấy đúng ID thuật toán từ thẻ
            int algoID = this->cards[i]->getId();
            std::cout << "START ALGO ID: " << algoID << std::endl;
            // TODO: Chuyển Scene sang VisualizeState tại đây
        };

        auto onBack = [this]() {
            this->expandedCard = nullptr;
        };

        // Gán Callbacks (Bắt buộc để nút hoạt động)
        cards[i]->setOnSelect(onSelect);
        cards[i]->setOnViewMore(onViewMore);
        cards[i]->setOnStart(onStart);
        cards[i]->setOnBack(onBack);

        cards[i]->handleEvent(event, window);
    }
}

void MenuState::update(float dt)
{
    updateLayout(dt);
}

void MenuState::draw()
{
    window.clear(currentWindowColor);
    window.pushGLStates();

    // Dùng Hằng số mới cho kích thước bảng vẽ
    float containerW = Theme::Style::AlgoBoardW; // 1200
    float containerH = Theme::Style::AlgoBoardH; // 580

    float startX = (window.getSize().x - containerW) / 2.f;
    float startY = (window.getSize().y - containerH) / 2.f + 30.f;

    if(!expandedCard)
    {
        window.draw(titleText);
        window.draw(subTitleText);
    }

    // --- VẼ BẢNG NỀN ---
    // Bóng đổ (Dùng AlgoRadius 48.0f)
    GUI::Squircle shadowRect({containerW, containerH}, Theme::Style::AlgoRadius);
    shadowRect.setCurvature(Theme::Style::SquircleCurvature);
    shadowRect.setPosition(startX + 10.f, startY + 10.f);
    shadowRect.setFillColor(Theme::Color::Shadow);
    window.draw(shadowRect);

    // Nền trắng
    GUI::Squircle bgRect({containerW, containerH}, Theme::Style::AlgoRadius);
    bgRect.setCurvature(Theme::Style::SquircleCurvature);
    bgRect.setPosition(startX, startY);
    bgRect.setFillColor(sf::Color::White);
    window.draw(bgRect);

    // --- SCISSOR TEST ---
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2i scissorPos = window.mapCoordsToPixel({startX, startY + containerH});
    sf::Vector2i scissorSize = window.mapCoordsToPixel({startX + containerW, startY}) - scissorPos;
    int scissorY = windowSize.y - scissorPos.y;

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorPos.x, scissorY, std::abs(scissorSize.x), std::abs(scissorSize.y));

    // Vẽ Cards
    for(auto c : cards)
    {
        if(c != expandedCard) c->draw(window);
    }
    if(expandedCard) expandedCard->draw(window);

    glDisable(GL_SCISSOR_TEST);
    window.popGLStates();
}
