#include "States/CategoriesState.h"
#include "GUI/MenuCard.h"
#include "MenuState.h"
#include "Easing.h"
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


//    float winW = (float)window.getSize().x;
//    float winH = (float)window.getSize().y;

    bgBoard.setCornerRadius(Theme::Style::CatRadius, Theme::Style::CatRadius, Theme::Style::CatRadius, Theme::Style::CatRadius);
    bgBoard.setSize({Theme::Style::CatBoardW, Theme::Style::CatBoardH});

    // Luôn căn giữa màn hình (hoặc lerp vị trí nếu tâm 2 bảng khác nhau)
    bgBoard.setOrigin(Theme::Style::CatBoardW / 2.0f, Theme::Style::CatBoardH / 2.0f);
    bgBoard.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f + 15.f);



    // Lambda tạo card
    // Note: col (màu) ở đây sẽ được set thành màu xám nhạt đồng nhất
    auto createCard = [&](int id, std::string name, std::string num) {
        sf::Color minimalistColor = sf::Color::White;
//        sf::Color minimalistColor = sf::Color::Black;
        auto* card = new GUI::MenuCard(id, name, num, minimalistColor);
        card->setSelectionStyle(false);

        // --- [MỚI] LOGIC TRANSITION (Thay cho goToAlgoList cũ) ---
        // Đây chính là onExpand mà ta đã bàn
        auto onTransitionStart = [this, card]()
        {
            if (this->isTransitioning) return;

            // --- 1. SETUP CHỮ ĐÓNG THẾ (PROXY TEXT) ---
            this->transitionText = card->getTitleText();
            this->transitionStartPos = card->getTitleText().getPosition();
            this->transitionText.setPosition(this->transitionStartPos);

            // Đích đến của chữ: Bay lên trên bảng mới 60px
            float targetW = Theme::Style::AlgoBoardW; // 1200
            float targetH = Theme::Style::AlgoBoardH; // 580
            float targetX = (window.getSize().x - targetW) / 2.0f;
            float targetY = (window.getSize().y - targetH) / 2.0f + 30.f;

            this->textTargetPos = { targetX, targetY - 60.0f };

            // Ẩn chữ thật đi
            card->setTextVisible(false);

//            // [MỚI] Ẩn nút View More bằng cách bỏ chọn thẻ
//            // Vì nút View More chỉ hiện khi Selected = true
//            card->setSelected(false);
//            // [QUAN TRỌNG] Ép cập nhật logic ẩn nút ngay lập tức (trước khi vẽ frame mới)
//            card->update(0.0f, window);
//            card->set

            // --- 2. DỌN DẸP CÁC CARD (Cho tất cả lui xuống) ---
            // Vì Bảng (Squircle) sẽ morph to ra, nên các card bên trong cần
            // mờ chữ và thu nhỏ lại để không gây rối mắt.
            for (auto c : this->cards)
            {
                // Fade Out chữ
                c->setTextOpacity(0.0f);
//                if(c == card) continue;

                // Scale Down (Thu nhỏ 20% về tâm)
                sf::FloatRect bounds = c->getGlobalBounds();
                sf::Vector2f center = {bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f};
                sf::Vector2f smallSize = {bounds.width * 0.8f, bounds.height * 0.8f};
                sf::Vector2f smallPos = {center.x - smallSize.x / 2.0f, center.y - smallSize.y / 2.0f};

                // MenuCard tự lo việc co nhỏ hình dáng của nó
                c->setTarget(smallPos, smallSize);
            }

            // --- 3. KÍCH HOẠT ---
            this->expandedCard = card;
            this->isTransitioning = true;
            this->transitionTimer = 0.0f;
            this->transitionDuration = Theme::Animation::HeroDuration;

            bgBoard.setCornerRadius(Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius);
        };

        // Gán logic mới vào nút View More
        card->setOnViewMore(onTransitionStart);

        // Logic highlight giữ nguyên
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
    isTransitioning = false;
    hasPushedState = false; // [THÊM] Reset cờ

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

    // [QUAN TRỌNG] Chặn input khi đang chuyển cảnh
    // Để chuột không vô tình hover vào thẻ làm nút View More hiện lại
    if (isTransitioning) return;

    // Truyền event vào từng card
    for(auto c : cards) c->handleEvent(event, window);
}

void CategoriesState::update(float dt)
{
    if(isTransitioning)
    {
        updateOutTransition(dt);
    }
    else
    {
        updateLayout(dt);
    }
}

void CategoriesState::draw()
{
    window.clear(Theme::Color::Background);

    // 1. Vẽ Bảng Nền (Luôn vẽ, vì nó đang morph to ra)
    window.draw(bgBoard);

    // 2. Vẽ Title tĩnh của màn hình Categories (Nằm dưới card)
    window.draw(titleText);
    window.draw(subTitleText);

    // 3. Vẽ các Card (Đang thu nhỏ lại trên nền bảng)
    for(auto c : cards) c->draw(window);

    // 4. Vẽ Chữ Đóng Thế (Bay trên cùng)
    if (isTransitioning) {
        window.draw(transitionText);
    }
}


void CategoriesState::updateOutTransition(float dt)
{
    transitionTimer += dt;

    // Tính toán tiến độ (0.0 -> 1.0)
    float t = std::min(transitionTimer / Theme::Animation::HeroDuration, 1.0f);
    // Dùng Easing Quart cho chuyển động mượt, đầm
    float ease = Utils::Math::Easing::easeOutQuart(t);

    // --- 1. MORPH BẢNG NỀN (Code tay phần này) ---
    float startW = Theme::Style::CatBoardW;
    float startH = Theme::Style::CatBoardH;
    float targetW = Theme::Style::AlgoBoardW;
    float targetH = Theme::Style::AlgoBoardH;

    // Nội suy kích thước
    float curW = Utils::Math::Easing::lerp(startW, targetW, ease);
    float curH = Utils::Math::Easing::lerp(startH, targetH, ease);

    // Cập nhật Squircle
    bgBoard.setSize({curW, curH});

    // Luôn căn giữa màn hình (hoặc lerp vị trí nếu tâm 2 bảng khác nhau)
    bgBoard.setOrigin(curW / 2.0f, curH / 2.0f);
    bgBoard.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f + 30.f);

    // --- 2. UPDATE CHỮ ĐÓNG THẾ (Bay lên) ---
    sf::Vector2f curTextPos;
    curTextPos.x = Utils::Math::Easing::lerp(transitionStartPos.x, textTargetPos.x, ease);
    curTextPos.y = Utils::Math::Easing::lerp(transitionStartPos.y, textTargetPos.y, ease);

    float curScale = Utils::Math::Easing::lerp(1.0f, 1.5f, ease);

    transitionText.setPosition(curTextPos);
    transitionText.setScale(curScale, curScale);

    // Đảm bảo alpha max
    sf::Color c = transitionText.getFillColor();
    c.a = 255;
    transitionText.setFillColor(c);

    // --- [MỚI] 3. FADE OUT TITLE & SUBTITLE ---
    // Tính alpha giảm dần từ 255 -> 0

    float fadeThreshold = 0.2f; // 30% thời gian
    float fadeT = std::min(t / fadeThreshold, 1.0f);

    float fadeAlpha = Utils::Math::Easing::lerp(255.0f, 0.0f, fadeT);

    // Áp dụng cho Title
    sf::Color tColor = titleText.getFillColor();
    tColor.a = (sf::Uint8)fadeAlpha;
    titleText.setFillColor(tColor);

    // Áp dụng cho SubTitle
    sf::Color sColor = subTitleText.getFillColor();
    sColor.a = (sf::Uint8)fadeAlpha;
    subTitleText.setFillColor(sColor);

    // --- 3. UPDATE CÁC CARD (Để chúng tự scale down và fade text) ---
    for (auto c : cards) {
        c->update(dt, window);
    }

    // --- 4. KẾT THÚC LOGIC ---
    if (transitionTimer >= transitionDuration)
    {
        // TRƯỜNG HỢP A: Chưa chuyển cảnh (Lần đầu chạy xong hiệu ứng)
        if (!hasPushedState)
        {
            // 1. Push MenuState
            int selectedID = expandedCard ? expandedCard->getId() : 0;
            states.push(new MenuState(window, states, selectedID));

            // 2. Đánh dấu
            hasPushedState = true;

            // 3. [QUAN TRỌNG] KHÔNG được set isTransitioning = false ở đây
            // Chúng ta cần giữ nó = true để khi quay lại, hàm này được gọi tiếp để chạy phần Reset bên dưới
            return;
        }

        // TRƯỜNG HỢP B: Đã chuyển cảnh xong và User vừa quay lại (Resume)
        // Lúc này mới thực hiện Reset
        else
        {
            // 1. Reset Bảng Nền
            bgBoard.setSize({Theme::Style::CatBoardW, Theme::Style::CatBoardH});
            bgBoard.setOrigin(Theme::Style::CatBoardW / 2.0f, Theme::Style::CatBoardH / 2.0f);
            bgBoard.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f + 30.f);

            // 2. Reset Text Title
            sf::Color tCol = titleText.getFillColor(); tCol.a = 255; titleText.setFillColor(tCol);
            sf::Color sCol = subTitleText.getFillColor(); sCol.a = 255; subTitleText.setFillColor(sCol);

            // 3. Reset Cards
            for (auto c : cards)
            {
                c->setTextVisible(true);
                c->setTextOpacity(-1); // Trả về mặc định
                c->setExpanded(false);
            }

            // 4. Snap Layout về vị trí cũ ngay lập tức
            updateLayout(0.0f);
//            for(auto c : cards) c->update(100.0f, window);

            // 5. Kết thúc Transition hoàn toàn
            isTransitioning = false;
            expandedCard = nullptr;
            hasPushedState = false; // Reset cờ cho lần sau
        }
    }
}
