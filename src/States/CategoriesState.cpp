#include "States/CategoriesState.h"
#include "GUI/MenuCard.h"
#include "MenuState.h"
#include "ViewHandler.h"
#include "Easing.h"
#include "ResourceManager.h"
#include "GUI/Squircle.h"
#include "WindowConfig.h"
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

    bgBoard.setRadius(Theme::Style::CatRadius, Theme::Style::CatRadius, Theme::Style::CatRadius, Theme::Style::CatRadius);
    bgBoard.setSize({Theme::Style::CatBoardW, Theme::Style::CatBoardH});

    // Luôn căn giữa màn hình (hoặc lerp vị trí nếu tâm 2 bảng khác nhau)
    bgBoard.setOrigin(Theme::Style::CatBoardW / 2.0f, Theme::Style::CatBoardH / 2.0f);
    bgBoard.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 15.f);



    shadowSprite.setTexture(res.getTexture("assets/textures/shadow_categories.png"));
    shadowSprite.setOrigin(Theme::Style::CatBoardW / 2.0f + 34.f, Theme::Style::CatBoardH / 2.0f + 30.f);
    shadowSprite.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 15.f);



    // Lambda tạo card
    // Note: col (màu) ở đây sẽ được set thành màu xám nhạt đồng nhất
    auto createCard = [&](int id, std::string name, std::string num, std::string shortDesc, std::string iconPath, sf::Color themeColor)
    {
        // Cấu hình Config
        GUI::CardConfig cfg;
        cfg.id = id;
        cfg.title = name;
        cfg.number = num;
        cfg.shortDescription = shortDesc;
        cfg.iconTexture = &res.getTexture(iconPath);
        cfg.themeColor = themeColor;
        cfg.initialSize = {Theme::Style::CatCardW, Theme::Style::CatCardH};
        // Nền trắng nên Title luôn phải là màu Tối, kể cả khi Selected hay Unselected
        cfg.colorTitleUnselected = sf::Color(29, 29, 31); // Xám than
        cfg.colorTitleSelected = sf::Color(29, 29, 31);   // Vẫn là Xám than (Hoặc màu đen)
        cfg.colorNumber = sf::Color(142, 142, 147);

//        sf::Color minimalistColor = sf::Color::Black;
        auto* card = new GUI::MenuCard(cfg);

        // --- [MỚI] LOGIC TRANSITION (Thay cho goToAlgoList cũ) ---
        // Đây chính là onExpand mà ta đã bàn
        auto onTransitionStart = [this, card]()
        {
            if (this->isTransitioning) return;

//            // 1. SETUP PROXY TEXT (CHỮ ĐÓNG THẾ)
//            // Lấy text thật từ thẻ
//            const sf::Text& srcText = card->getTitleText();

//            // Copy toàn bộ thuộc tính sang transitionText
//            this->transitionText = srcText;

//            // Lấy vị trí thế giới (World Position) của chữ thật
//            // Vì MenuCard::updateLayout đã setPosition tuyệt đối cho textTitle, nên ta lấy trực tiếp được
//            this->transitionStartPos = srcText.getPosition();

//            // Đặt chữ đóng thế vào đúng vị trí chữ thật
//            this->transitionText.setPosition(this->transitionStartPos);

//            // Đảm bảo chữ đóng thế hiện rõ 100% (Alpha 255)
//            sf::Color c = this->transitionText.getFillColor();
//            c.a = 255;
//            this->transitionText.setFillColor(c);


//            // 2. TÍNH TOÁN ĐÍCH ĐẾN (Target Position)
//            // Chữ sẽ bay đến tiêu đề của bảng Algorithms (MenuState)
//            // Vị trí này nằm ở trên cùng của bảng Algo (1200x580)
//            float targetBoardY = (window.getSize().y - Theme::Style::AlgoBoardH) / 2.0f;
//
//            // Đích đến: Giữa màn hình (X), Trên đầu bảng Algo một chút (Y)
//            this->textTargetPos.x = window.getSize().x / 2.0f;
//            this->textTargetPos.y = targetBoardY + 40.0f; // Padding top 40px
//
//            // Đặt Origin chữ đóng thế vào giữa để căn chỉnh chuẩn
//            sf::FloatRect b = this->transitionText.getLocalBounds();
//            this->transitionText.setOrigin(b.left + b.width/2.0f, b.top + b.height/2.0f);



//            // --- 1. SETUP CHỮ ĐÓNG THẾ (PROXY TEXT) ---
//            this->transitionText = card->getTitleText();
//            this->transitionStartPos = card->getTitleText().getPosition();
//            this->transitionText.setPosition(this->transitionStartPos);
//
//            // Đích đến của chữ: Bay lên trên bảng mới 60px
//            float targetW = Theme::Style::AlgoBoardW; // 1200
//            float targetH = Theme::Style::AlgoBoardH; // 580
//            float targetX = (window.getSize().x - targetW) / 2.0f;
//            float targetY = (window.getSize().y - targetH) / 2.0f + 30.f;
//
//            this->textTargetPos = { targetX, targetY - 60.0f };


            // 3. ẨN CARD THẬT & KÍCH HOẠT HIỆU ỨNG
            for (auto c : this->cards)
            {
                // Làm mờ toàn bộ thẻ (bao gồm cả chữ thật) -> Chữ đóng thế sẽ hiện ra thay thế
                c->setOpacity(0.0f);

                // Thu nhỏ các thẻ về tâm (hiệu ứng phụ)
                sf::FloatRect bounds = c->getGlobalBounds();
                sf::Vector2f center = {bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f};
                sf::Vector2f smallSize = {bounds.width * 0.8f, bounds.height * 0.8f};
                c->setTarget(center, smallSize);
            }

            // --- 3. KÍCH HOẠT ---
            this->expandedCard = card;
            this->isTransitioning = true;
            this->transitionTimer = 0.0f;
            this->transitionDuration = Theme::Animation::CategoriesOutDuration;

            bgBoard.setRadius(Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius);
        };

        // Gán logic mới vào nút View More
        card->onViewMore = onTransitionStart;

        // Logic highlight giữ nguyên
        card->onSelect = [this, id]() {
            this->selectedIndex = id;
        };

        cards.push_back(card);
    };

    sf::Color themeColor = sf::Color(255, 255, 255, 0);

    // Tạo các thẻ với cùng một màu nền (loại bỏ màu xanh, cam, tím)
    createCard(0, "Linear Structures", "01", "Sequential flow.", "assets/textures/linear_structures_icon.png", themeColor);
    createCard(1, "Hierarchical Structures",   "02", "Branched logic.", "assets/textures/hierarchical_structures_icon.png", themeColor);
    createCard(2, "Graphs Structures",     "03", "Connected nodes.", "assets/textures/graphs_structures_icon.png", themeColor);

    selectedIndex = -1;
    isTransitioning = false;
    hasPushedState = false; // [THÊM] Reset cờ

    // Cập nhật bố cục ngay lập tức
    updateLayout(0.0f);
}

void CategoriesState::updateLayout(float dt)
{
    float winW = Utils::System::DESIGN_WIDTH;
    float winH = Utils::System::DESIGN_HEIGHT;

    float containerW = Theme::Style::CatBoardW;
    float containerH = Theme::Style::CatBoardH;

    // Vị trí tâm của bảng
    float centerX = winW / 2.0f;
    float centerY = winH / 2.0f + 15.f; // Offset Y tí cho đẹp

    // Update Text (Giữ nguyên)
    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;
    titleText.setPosition(startX, startY - 80.f);
    subTitleText.setPosition(startX, startY - 40.f);

    // LOGIC CHIA CỘT MỚI (Dựa trên tâm)
    // Categories có 3 thẻ.
    // Thẻ 1: Tâm bảng - 320px
    // Thẻ 2: Tâm bảng
    // Thẻ 3: Tâm bảng + 320px

    float cardW = Theme::Style::CatCardW; // 320
    float cardH = Theme::Style::CatCardH; // 460

    for(int i = 0; i < (int)cards.size(); ++i)
    {
        // Tính xOffset từ tâm (-1, 0, 1)
        float offsetIndex = (float)i - (float)(cards.size() - 1) / 2.0f;
        float targetX = centerX + (offsetIndex * cardW);
        float targetY = centerY;

        // [SỬA] setTarget: Pos (Tâm) và Size
        cards[i]->setTarget({targetX, targetY}, {cardW, cardH});

        // [SỬA] Logic Bo Góc (Dùng Enum mới)
        GUI::CardPos pos = GUI::CardPos::Middle;
        if (i == 0) pos = GUI::CardPos::First;
        else if (i == (int)cards.size() - 1) pos = GUI::CardPos::Last;

        cards[i]->setCardPosition(pos); // Không cần truyền radius nữa

        // Highlight logic
        cards[i]->setSelected(i == selectedIndex);

        // Update
        cards[i]->update(dt, window);
    }
}

void CategoriesState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Closed) window.close();

    // THÊM ĐOẠN NÀY ĐỂ TẠO VIỀN ĐEN KHI CO GIÃN
    if (event.type == sf::Event::Resized)
    {
        Utils::System::applyLetterboxView(window, event.size.width, event.size.height);
    }

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

//    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
//    {
//        // xử lý
//        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
//        if(!bgBoard.getGlobalBounds().contains(mouse))
//        {
//            expandedCard = nullptr;
//            selectedIndex = -1;
//        }
//    }

    if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2i pixelPos(event.mouseButton.x, event.mouseButton.y);

        if(!Utils::ViewHandler::isMouseInFrame(pixelPos, window, bgBoard.getGlobalBounds()))
        {
            expandedCard = nullptr;
            selectedIndex = -1;
        }
    }



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

    if(!isTransitioning || 1) window.draw(shadowSprite);

    // 1. Vẽ Bảng Nền (Luôn vẽ, vì nó đang morph to ra)
//        bgBoard.setFillColor(sf::Color::Magenta); ///********************************
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
    float t = std::min(transitionTimer / Theme::Animation::CategoriesOutDuration, 1.0f);
    // Dùng Easing Quart cho chuyển động mượt, đầm
    float ease = Utils::Math::Easing::easeOutCubic(t);

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

    float ratioW = curW / Theme::Style::CatBoardW, ratioH = curH / Theme::Style::CatBoardH;

    shadowSprite.setColor(sf::Color::Red);

    shadowSprite.setScale(sf::Vector2f(ratioW, ratioH));
    shadowSprite.setOrigin(Theme::Style::CatBoardW / 2.0f + 34.f, Theme::Style::CatBoardH / 2.0f + 30.f);
    shadowSprite.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 30.f);

    // Luôn căn giữa màn hình (hoặc lerp vị trí nếu tâm 2 bảng khác nhau)
    bgBoard.setOrigin(curW / 2.0f, curH / 2.0f);
    bgBoard.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 30.f);

//    // --- 2. UPDATE CHỮ ĐÓNG THẾ (Bay lên) ---
//    sf::Vector2f curPos;
//    curPos.x = Utils::Math::Easing::lerp(transitionStartPos.x, textTargetPos.x, ease);
//    curPos.y = Utils::Math::Easing::lerp(transitionStartPos.y, textTargetPos.y, ease);

//    float curScale = Utils::Math::Easing::lerp(1.0f, 1.5f, ease);

//    transitionText.setPosition(curPos);
//    transitionText.setScale(curScale, curScale);

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
            bgBoard.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 15.f);

            shadowSprite.setScale(1.f, 1.f);
            shadowSprite.setOrigin(Theme::Style::CatBoardW / 2.0f + 34.f, Theme::Style::CatBoardH / 2.0f + 30.f);
            shadowSprite.setPosition(Utils::System::DESIGN_WIDTH / 2.0f, Utils::System::DESIGN_HEIGHT / 2.0f + 15.f);

            // 2. Reset Text Title
            sf::Color tCol = titleText.getFillColor(); tCol.a = 255; titleText.setFillColor(tCol);
            sf::Color sCol = subTitleText.getFillColor(); sCol.a = 255; subTitleText.setFillColor(sCol);

            // 3. [SỬA] Reset Cards
            for (auto c : cards)
            {
                // Trả lại độ đậm
                c->setOpacity(255.0f);
                // Tắt trạng thái expand
                c->setExpanded(false);
                // Tắt trạng thái chọn (để nút View More ẩn đi)
                c->setSelected(false);
            }

//            selectedIndex = -1;

            // 4. Snap Layout về vị trí cũ ngay lập tức
            updateLayout(0.0f);
            for(auto c : cards) c->snapToTarget();

            // 5. Kết thúc Transition hoàn toàn
            isTransitioning = false;
            expandedCard = nullptr;
            hasPushedState = false; // Reset cờ cho lần sau
        }
    }
}
