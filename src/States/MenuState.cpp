#include "MenuState.h"
#include "MenuCard.h"
#include "ResourceManager.h"
#include "Squircle.h" // <--- Đổi include này
#include "Smoothing.h"
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <algorithm>

// Alias cho gọn
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

MenuState::MenuState(sf::RenderWindow& window, std::stack<State*>& states)
    : window(window), states(states)
{
    init();
}

MenuState::~MenuState()
{
    for(auto card : cards) delete card;
    cards.clear();
}

void MenuState::init()
{
    auto& res = ResourceManager::getInstance();
    sf::Font& fontHeader = res.getFont("assets/fonts/font.ttf");

    // --- 1. SETUP HEADER ---
    titleText.setFont(fontHeader);
    titleText.setString("Data Structures");
    titleText.setCharacterSize(34);
    titleText.setFillColor(Theme::Color::TextPrimary); // Dùng màu Theme

    subTitleText.setFont(res.getFont("assets/fonts/font.ttf"));
    subTitleText.setString("Select to visualize");
    subTitleText.setCharacterSize(18);
    subTitleText.setFillColor(Theme::Color::TextSecondary); // Dùng màu Theme

    // --- 2. CREATE CARDS ---
    // Giữ nguyên màu riêng biệt cho từng thuật toán để dễ phân biệt
    cards.push_back(new GUI::MenuCard(0, "Linked List", "01", sf::Color(255, 107, 107)));
    cards.push_back(new GUI::MenuCard(1, "Binary Heap", "02", sf::Color(255, 159, 67)));
    cards.push_back(new GUI::MenuCard(2, "AVL Tree",    "03", sf::Color(84, 160, 255)));
    cards.push_back(new GUI::MenuCard(3, "Graph Algo",  "04", sf::Color(95, 39, 205)));

    // --- 3. INIT STATES ---
    selectedIndex = -1;
    expandedCard = nullptr;
    entryTimer = 0.0f;

    currentWindowColor = Theme::Color::Background; // Dùng màu Theme

    updateLayout(0.0f);
}

void MenuState::updateLayout(float dt)
{
    float winW = (float)window.getSize().x;
    float winH = (float)window.getSize().y;

    // --- 1. DÙNG HẰNG SỐ TỪ THEME ---
    float containerW = Theme::Style::MenuWidth;  // 1100.f
    float containerH = Theme::Style::MenuHeight; // 550.f

    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;

    titleText.setPosition(startX, startY - 80.f);
    subTitleText.setPosition(startX, startY - 40.f);

    // --- 2. BACKGROUND COLOR LOGIC ---
    sf::Color targetWinColor = Theme::Color::Background;

    if(expandedCard)
    {
        targetWinColor = getPaleColor(expandedCard->getThemeColor());
    }

    // Dùng tốc độ từ Theme
    currentWindowColor = Utils::Math::Smoothing::dampColor(
        currentWindowColor,
        targetWinColor,
        Theme::Animation::BgSmoothing, // 0.2f
        dt,
        Theme::Animation::BgMinSpeed   // 10.0f
    );

    // --- 3. CARD LOGIC ---
    if(expandedCard != nullptr)
    {
        // MODE: EXPANDED
        for(auto c : cards)
        {
            if(c == expandedCard)
            {
                // Card mở rộng chiếm toàn bộ bảng
                c->setTarget({startX, startY}, {containerW, containerH});
                c->setExpanded(true);
            }
            else
            {
                // Các card khác trượt ra ngoài
                float offX = (c->getId() < expandedCard->getId()) ? startX - 300.f : startX + containerW + 300.f;
                // Kích thước khi trượt đi không quan trọng lắm vì nó ẩn, nhưng set cho hợp lý
                c->setTarget({offX, startY}, {containerW / 4.f, containerH});
                c->setExpanded(false);
            }
        }
    }
    else
    {
        // MODE: ACCORDION
        float w = containerW / cards.size();
        float currentX = startX;

        // Bán kính bo góc cho item accordion (24.0f)
        float cornerRadius = Theme::Style::ItemRadius;

        for(int i = 0; i < (int)cards.size(); ++i)
        {
            cards[i]->setTarget({currentX, startY}, {w, containerH});
            cards[i]->setSelected(i == selectedIndex);
            cards[i]->setExpanded(false);

            GUI::CardPos pos = GUI::CardPos::Middle;
            if(i == 0) pos = GUI::CardPos::First;
            else if(i == cards.size() - 1) pos = GUI::CardPos::Last;

            cards[i]->setCardPosition(pos, cornerRadius);

            currentX += w;
        }
    }

    for(auto c : cards)
    {
        c->update(dt, window);
    }
}

void MenuState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Closed) window.close();

    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        if(expandedCard) expandedCard = nullptr;
        else window.close();
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
            std::cout << "START ID: " << i << std::endl;
            // TODO: Chuyển Scene sang VisualizeState tại đây
        };

        auto onBack = [this]() {
            this->expandedCard = nullptr;
        };

        // Truyền window (đã bỏ const trong MenuCard)
        cards[i]->handleEvent(event, window, onSelect, onViewMore, onStart, onBack);
    }
}

void MenuState::update(float dt)
{
    updateLayout(dt);
}

void MenuState::draw()
{
    window.clear(currentWindowColor);

    // Save GL State
    window.pushGLStates();

    float winW = (float)window.getSize().x;
    float winH = (float)window.getSize().y;

    // Dùng Theme Constants
    float containerW = Theme::Style::MenuWidth;
    float containerH = Theme::Style::MenuHeight;

    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;

    if(!expandedCard)
    {
        window.draw(titleText);
        window.draw(subTitleText);
    }

    // --- VẼ BẢNG NỀN & BÓNG (SQUIRCLE) ---

    // 1. Bóng đổ
    GUI::Squircle shadowRect({containerW, containerH}, Theme::Style::CardRadius); // Radius 48
    shadowRect.setCurvature(Theme::Style::SquircleCurvature); // Curvature 4.0
    shadowRect.setPosition(startX + 10.f, startY + 10.f);
    shadowRect.setFillColor(Theme::Color::Shadow);
    window.draw(shadowRect);

    // 2. Nền trắng
    GUI::Squircle bgRect({containerW, containerH}, Theme::Style::CardRadius);
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

    // Restore GL State
    window.popGLStates();
}
