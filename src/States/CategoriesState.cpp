#include "States/CategoriesState.h"
#include "GUI/MenuCard.h"
#include "MenuState.h"
#include "ResourceManager.h"
#include "GUI/Squircle.h"
#include "SFML/Graphics.hpp"
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <cmath>

namespace Theme = Utils::Graphics::Theme;

CategoriesState::CategoriesState(sf::RenderWindow& window, std::stack<State*>& states)
    : window(window), states(states)
{
    init();
}

CategoriesState::~CategoriesState()
{
    for(auto c : cards) delete c;
    cards.clear();
}

void CategoriesState::init()
{
    auto& res = ResourceManager::getInstance();
    sf::Font& fontHeader = res.getFont("assets/fonts/font.ttf");

    // Setup Title
    titleText.setFont(fontHeader);
    titleText.setString("Data Structures");
    titleText.setCharacterSize(34);
    titleText.setFillColor(Theme::Color::TextPrimary); // Màu chữ chính (Đen/Xám đậm)

    // Setup Subtitle
    subTitleText.setFont(fontHeader);
    subTitleText.setString("Select Category");
    subTitleText.setCharacterSize(18);
    subTitleText.setFillColor(Theme::Color::TextSecondary); // Màu chữ phụ (Xám nhạt hơn)

    // Lambda tạo card
    // Note: col (màu) ở đây sẽ được set thành màu xám nhạt đồng nhất
    auto createCard = [&](int id, std::string name, std::string num) {
        // Sử dụng màu xám rất nhạt (gần như trắng) cho phong cách minimalism
        // Bạn có thể đổi thành sf::Color::White nếu muốn trắng hoàn toàn
        sf::Color minimalistColor = sf::Color(248, 248, 248);

        auto* card = new GUI::MenuCard(id, name, num, minimalistColor);

        // --- LOGIC CHUYỂN CẢNH ---
        auto goToAlgoList = [this, id]() {
            std::cout << "Switching to Category ID: " << id << std::endl;
            this->states.push(new MenuState(this->window, this->states, id));
        };

        // Gán logic vào nút View More
        card->setOnViewMore(goToAlgoList);

        // Sự kiện click vào thẻ để highlight
        card->setOnSelect([this, id]() {
            this->selectedIndex = id;
        });

        cards.push_back(card);
    };

    // Tạo các thẻ với cùng một màu nền (loại bỏ màu xanh, cam, tím)
    createCard(0, "Linear Structures", "01");
    createCard(1, "Tree Structures",   "02");
    createCard(2, "Advanced Algo",     "03");

    selectedIndex = -1;

    // Cập nhật bố cục ngay lập tức
    updateLayout(0.0f);
}

void CategoriesState::updateLayout(float dt)
{
    float winW = (float)window.getSize().x;
    float winH = (float)window.getSize().y;

    // Kích thước bảng lấy từ Theme (theme.h)
    float containerW = Theme::Style::CatBoardW;
    float containerH = Theme::Style::CatBoardH;

    // Vị trí giữa màn hình
    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;

    // Update vị trí Text
    titleText.setPosition(startX, startY - 80.f);
    subTitleText.setPosition(startX, startY - 40.f);

    // Logic chia cột (Layout tĩnh, không co giãn accordion để giữ sự đơn giản)
    float cardW = containerW / (float)cards.size();
    float currentX = startX;

    // Radius bo góc
    float r = Theme::Style::CatRadius;

    for(int i = 0; i < (int)cards.size(); ++i)
    {
        // Set vị trí và kích thước cho thẻ
        cards[i]->setTarget({currentX, startY}, {cardW, containerH});

        // Logic bo góc (Chỉ thẻ đầu và cuối mới cần bo góc phía ngoài)
        GUI::CardPos pos = GUI::CardPos::Middle;
        if(i == 0) pos = GUI::CardPos::First;
        else if(i == (int)cards.size() - 1) pos = GUI::CardPos::Last;

        cards[i]->setCardPosition(pos, r);

        // Highlight logic (Chỉ để xác định card nào đang được chọn, không đổi màu nền app)
        cards[i]->setSelected(i == selectedIndex);

        // Update card
        cards[i]->update(dt, window);

        currentX += cardW;
    }
}

void CategoriesState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Closed) window.close();

//    // Logic Hover cơ bản
//    sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
//    selectedIndex = -1;
//    for(int i = 0; i < (int)cards.size(); ++i)
//    {
//        if(cards[i]->getGlobalBounds().contains(mPos))
//        {
////            selectedIndex = i;
//            break;
//        }
//    }

    // Truyền event vào từng card
    for(auto c : cards) c->handleEvent(event, window);
}

void CategoriesState::update(float dt)
{
    updateLayout(dt);
}

void CategoriesState::draw()
{
    // Luôn sử dụng màu nền cố định từ Theme (không pha màu)
    window.clear(Theme::Color::Background);

    // Save state để dùng Scissor
    window.pushGLStates();

    // 1. Vẽ Title & Subtitle
    window.draw(titleText);
    window.draw(subTitleText);

    // --- VẼ KHỐI BẢNG (Minimalist Style) ---

    float containerW = Theme::Style::CatBoardW;
    float containerH = Theme::Style::CatBoardH;
    float startX = (window.getSize().x - containerW) / 2.f;
    float startY = (window.getSize().y - containerH) / 2.f + 30.f;

    // A. Vẽ Bóng Đổ (Shadow nhẹ nhàng)
    GUI::Squircle shadow({containerW, containerH}, Theme::Style::CatRadius);
    shadow.setCurvature(Theme::Style::SquircleCurvature);
    shadow.setFillColor(Theme::Color::Shadow); // Màu bóng (thường là xám trong suốt)
    shadow.setPosition(startX + 10.f, startY + 10.f);
    window.draw(shadow);

    // B. Vẽ Bảng Nền Trắng
    GUI::Squircle bgRect({containerW, containerH}, Theme::Style::CatRadius);
    bgRect.setCurvature(Theme::Style::SquircleCurvature);
    bgRect.setFillColor(sf::Color::White); // Nền bảng luôn trắng
    bgRect.setPosition(startX, startY);
    window.draw(bgRect);

    // C. Cắt Scissor (Khuôn đúc để nội dung không tràn ra ngoài bo góc)
    sf::Vector2u winSize = window.getSize();
    sf::Vector2i scissorPos = window.mapCoordsToPixel({startX, startY + containerH});
    sf::Vector2i scissorSize = window.mapCoordsToPixel({startX + containerW, startY}) - scissorPos;
    int scissorY = winSize.y - scissorPos.y;

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorPos.x, scissorY, std::abs(scissorSize.x), std::abs(scissorSize.y));

    // D. Vẽ Cards
    for(auto c : cards)
    {
        c->draw(window);
    }

    glDisable(GL_SCISSOR_TEST);

    window.popGLStates();
}
