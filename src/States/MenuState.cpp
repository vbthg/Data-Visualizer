#include "MenuState.h"
#include "MenuCard.h"
#include "ResourceManager.h"
#include "Squircle.h"
#include "WindowConfig.h"
#include "Smoothing.h"
#include "Easing.h"
#include "Theme.h"
#include "ViewHandler.h"
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <algorithm>
#include <random>    // Để dùng std::random_device, std::mt19937
#include <VisualizerState.h>
#include <DataStructure.h>
//#include <SinglyLinkedList.h>
#include <Heap.h>
#include <cassert>

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
    cat1.algos = {
        {
            0,
            "Singly Linked List",
            "01",
            "A streamlined, forward-only data chain. Engineered for lightweight, linear traversal with absolute simplicity.",
            "Forward flow.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            1,
            "Doubly Linked List",
            "02",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            2,
            "Stack & Queue",
            "03",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            3,
            "Circular List",
            "04",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        }
    };

    allData.push_back(cat1);

    // ID 1: Tree
    CategoryInfo cat2; cat2.name = "Tree Structures"; cat2.color = sf::Color(255, 159, 67);
    cat2.algos = {
        {
            4,
            "Binary Heap",
            "05",
            "A tree-based priority system. Instantly surfaces the most critical element with zero friction. Fast, optimized, and uncompromising.",
            "Priority-first logic.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            5,
            "AVL Tree",
            "06",
            "A perfectly balanced binary search tree. Automatically rotates after every action to maintain optimal query speeds at all times.",
            "Self-balancing precision.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            6,
            "Red-Black Tree",
            "07",
            "A harmony of color rules and branching logic. Guarantees rock-solid performance and stability in the most demanding environments.",
            "Color-coded efficiency.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            7,
            "2-3-4 Tree",
            "08",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        }
    };
    allData.push_back(cat2);

    // ID 2: Advanced
    CategoryInfo cat3; cat3.name = "Graphs Structures"; cat3.color = sf::Color(95, 39, 205);
    cat3.algos = {
        {
            8,
            "Minimum Spanning Tree",
            "09",
            "Powered by Kruskal's algorithm. Connects the entire network with zero gaps, zero cycles, and the absolute minimum resource cost.",
            "Optimal connectivity.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            9,
            "Graph (BFS/DFS)",
            "10",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            10,
            "Shortest Path",
            "11",
            "Powered by Dijkstra's algorithm. Scans every branch to pinpoint the exact shortest route. Unrivaled speed for precise navigation.",
            "The fastest route.",
            "assets/textures/singly_linked_list_icon.png"
        },

        {
            11,
            "Dijkstra",
            "12",
            "abc",
            "short desc",
            "assets/textures/singly_linked_list_icon.png"
        }
    };
    allData.push_back(cat3);
}

void MenuState::init()
{
    isTransitioning = true;
    transitionTimer = 0;

    // 1. Load Data
    initData();
    if (currentCategory < 0 || currentCategory >= (int)allData.size()) currentCategory = 0;
    CategoryInfo& info = allData[currentCategory];

    auto& res = ResourceManager::getInstance();
    sf::Font& fontHeader = res.getFont("assets/fonts/font.ttf");

    // --- SETUP HEADER (Theo tên Category) ---
    // Header Metrics (Chỉ để căn chỉnh vị trí chữ)
    float headerW = 1200.0f;
    float headerH = 80.0f;
    float headerX = (Utils::System::DESIGN_WIDTH - headerW) / 2.0f;
    float headerY = 30.0f;

    titleText.setFont(fontHeader);
    titleText.setString(info.name); // <--- Lấy tên từ dữ liệu
    titleText.setCharacterSize(34);
    titleText.setFillColor(Theme::Color::TextPrimary);

    // Đặt Origin: Cạnh trái, Giữa theo chiều dọc (Middle-Left) -> Để dễ canh với Icon
    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setOrigin(tb.left, tb.top + tb.height/2.0f);

    // Đặt vị trí: Cách lề 50px để chừa chỗ cho mũi tên
    titleText.setPosition(headerX + 50.0f, headerY + 40.0f);

    subTitleText.setFont(res.getFont("assets/fonts/font.ttf"));
    subTitleText.setString("Select to visualize");
    subTitleText.setCharacterSize(18);
    subTitleText.setFillColor(Theme::Color::TextSecondary);


    // --- B. SETUP BACK ICON (Thêm mới) ---
    fontIcon = res.getFont("assets/fonts/phosphor.ttf");

    backIcon.setFont(fontIcon);
    backIcon.setString(L"\ue138"); // Mã Unicode của CaretLeft trong Phosphor
    backIcon.setCharacterSize(28);
    backIcon.setFillColor(Theme::Color::TextPrimary);

    // Đặt Origin: Chính giữa Icon (Center)
    sf::FloatRect ib = backIcon.getLocalBounds();
    backIcon.setOrigin(ib.left + ib.width/2.0f, ib.top + ib.height/2.0f);

    // Đặt vị trí: Nằm bên trái Text (cách lề 20px)
    backIcon.setPosition(headerX + 20.0f, headerY + 40.0f);


    // --- C. SETUP GHOST BUTTON (Controller) ---
    // Tính kích thước nút sao cho bao trọn cả Icon và Text
    float btnWidth = 50.0f + tb.width + 20.0f; // 50px (vùng Icon) + Text + Padding đuôi
    float btnHeight = 60.0f;

    // Tạo nút rỗng (Text rỗng)
    btnBack = new GUI::Button(fontHeader, "", {btnWidth, btnHeight});

    // Preset Ghost để trong suốt (nhưng vẫn bắt được chuột)
    btnBack->applyPreset(GUI::ButtonPreset::Ghost);
    btnBack->setCornerRadius(12.0f);

    btnBack->setPosition({headerX + btnWidth/2.0f, headerY + 40.0f});

    // Logic Click (Giữ nguyên)
    btnBack->onClick = [this]() {
        if (this->expandedCard) this->expandedCard = nullptr;
        else this->states.pop();
    };


//    btnBack = new GUI::Button(fontIcon, L"\ue902", {48.f, 48.f});
//    btnBack->applyPreset(GUI::ButtonPreset::Ghost);
//    btnBack->setCharacterSize(28); // Kích thước Icon
//
//    // Logic Click Nút Back
//    btnBack->onClick = [this]() {
//        if (this->expandedCard) this->expandedCard = nullptr;
//        else this->states.pop();
//    };
//    btnBack->setPosition({headerX, headerY + 40.0f});

    // 4. INIT STATIC BOARD (Bảng nền cố định)

    float containerW = Theme::Style::AlgoBoardW; // 1200.0f
    float containerH = Theme::Style::AlgoBoardH; // 580.0f

    // Tính toán vị trí bắt đầu
    float startX = (Utils::System::DESIGN_WIDTH - containerW) / 2.0f;
    float startY = (Utils::System::DESIGN_HEIGHT - containerH) / 2.0f + 30.f; ///*****************

    bgBoard.setSize({containerW, containerH});
    bgBoard.setRadius(Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius, Theme::Style::AlgoRadius); // 48.0f
    bgBoard.setFillColor(sf::Color::White);
    bgBoard.setPosition(startX, startY);
//    bgBoard.setCurvature(Theme::Style::SquircleCurvature); // Siêu elip cho mượt


    shadowSprite.setTexture(res.getTexture("assets/textures/shadow_algo.png"));
    shadowSprite.setOrigin(34.f, 30.f);
    shadowSprite.setPosition(startX, startY);
//    shadowSprite.setColor(sf::Color(255,255,255,0));




    // --- 1. CHUẨN BỊ BẢNG MÀU (PALETTE) ---
    // Danh sách 8 màu Pastel dịu mắt, đảm bảo khác nhau
    std::vector<sf::Color> palette = {
        sf::Color(255, 69, 58),   // Red (Đỏ san hô đậm)
        sf::Color(255, 159, 10),  // Orange (Cam Apple)
        sf::Color(48, 209, 88),   // Green (Xanh lá tươi)
        sf::Color(10, 132, 255),  // Blue (Xanh dương đặc trưng iOS)
        sf::Color(94, 92, 230),   // Indigo (Tím thẫm)
        sf::Color(191, 90, 242),  // Purple (Tím mộng mơ)
        sf::Color(255, 55, 95),   // Pink (Hồng neon dịu)
        sf::Color(0, 199, 190)    // Teal (Xanh ngọc bích)
    };

    // --- 2. XÁO TRỘN MÀU (SHUFFLE) ---
    // Dùng random device để mỗi lần mở lên là một thứ tự khác nhau
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(palette.begin(), palette.end(), g);



    // --- CREATE CARDS (Theo danh sách thuật toán) ---
    // Xóa thẻ cũ (nếu có)
    for(auto c : cards) delete c;
    cards.clear();

    float cardW = containerW / info.algos.size();
    float currentX = startX;

    // Biến đếm để xác định vị trí đầu/cuối
    int index = 0;
    int totalCount = (int)info.algos.size();

    const std::string filepath[3][4] = {
        {
            "assets/textures/heap_icon.png",
            "assets/textures/singly_linked_list_icon.png",
            "assets/textures/AVL_tree_icon.png",
            "assets/textures/B-tree_icon.png"
        },
        {
            "assets/textures/mst_icon.png",
            "assets/textures/shortest_path_icon.png",
            "assets/textures/singly_linked_list_icon.png",
            "assets/textures/singly_linked_list_icon.png"
        },
        {
            "assets/textures/singly_linked_list_icon.png",
            "assets/textures/singly_linked_list_icon.png",
            "assets/textures/singly_linked_list_icon.png",
            "assets/textures/singly_linked_list_icon.png"
        }
    };

    int curId = -1;

    // Tạo thẻ động
    for (const auto& algo : info.algos)
    {
        ++curId;
        // [SỬA 1] Cấu hình Config thay vì truyền tham số lẻ tẻ
        GUI::CardConfig cfg;
        cfg.id = algo.id;
        cfg.title = algo.name;
        cfg.number = algo.number;

        cfg.description = algo.description; // Truyền data từ Model sang View
        cfg.shortDescription = algo.shortDescription;
        cfg.iconTexture = &res.getTexture(algo.iconPath); // Load icon tương ứng

        // Nền đậm nên Title Unselected là Đen/Xám, Selected là Trắng
        cfg.colorTitleUnselected = sf::Color(29, 29, 31);
        cfg.colorTitleSelected = sf::Color(255, 255, 255);

        // Tạm thời chưa có icon cho từng thuật toán, ta dùng icon mặc định hoặc thêm logic icon
        // Ví dụ: lấy icon từ Category hoặc Placeholder
        // cfg.iconTexture = &res.getTexture("assets/icons/default_algo.png");
        // Nếu chưa có texture, bạn có thể null check trong MenuCard hoặc trỏ tạm vào 1 texture nào đó
        // Giả sử ta lấy texture đầu tiên của category làm mẫu
//        if (currentCategory == 0) cfg.iconTexture = &res.getTexture("assets/icons/linear.png");
//        else if (currentCategory == 1) cfg.iconTexture = &res.getTexture("assets/icons/tree.png");
//        else cfg.iconTexture = &res.getTexture("assets/icons/graph.png");
//        cfg.iconTexture = &res.getTexture(filepath[currentCategory][curId]);

        cfg.themeColor = palette[index % palette.size()];
        cfg.colorNumber = cfg.themeColor;

        // Kích thước khởi tạo của thẻ Algo
        cfg.initialSize = {Theme::Style::AlgoCardW, Theme::Style::AlgoCardH};


        // [SỬA 2] New MenuCard với config
        cards.push_back(new GUI::MenuCard(cfg));
        auto c = cards.back();


        // [SỬA 3] Logic setCardPosition (Chỉ truyền Enum, không truyền radius)
        GUI::CardPos pos = GUI::CardPos::Middle;
        if (totalCount == 1) pos = GUI::CardPos::Single;
        else if (index == 0) pos = GUI::CardPos::First;
        else if (index == totalCount - 1) pos = GUI::CardPos::Last;

        c->setCardPosition(pos); // Bỏ tham số radius đi


        // [SỬA 4] Set Target ban đầu (Hiệu ứng trượt lên)
        // Vị trí thấp hơn bình thường để trượt lên
        float startY_Offset = startY + 100.0f;

        // Lưu ý: setTarget nhận vị trí TÂM (Center), không phải Top-Left
        // startX + index*cardW là Top-Left của cột
        // Tâm X = TopLeftX + Width/2
        float centerX = currentX + cardW/2.0f;
        float centerY = startY_Offset + containerH/2.0f;

        c->setTarget({centerX, centerY}, {cardW, containerH});

        // [SỬA 5] Dùng setOpacity thay vì setTextOpacity
        c->setOpacity(0.0f);

        // Teleport
        c->snapToTarget(); // Dùng hàm mới snapToTarget

        currentX += cardW;
        index++; // Đừng quên tăng index



    }

    // --- INIT STATES ---
    selectedIndex = -1;
    expandedCard = nullptr;
    entryTimer = 0.0f;

    currentWindowColor = Theme::Color::Background;

//    updateLayout(0.0f);

//    exit(0);
}

void MenuState::updateLayout(float dt)
{
    float winW = Utils::System::DESIGN_WIDTH;
    float winH = Utils::System::DESIGN_HEIGHT;

    // --- 1. DÙNG HẰNG SỐ MỚI TỪ THEME ---
    float containerW = Theme::Style::AlgoBoardW; // 1200.0f
    float containerH = Theme::Style::AlgoBoardH; // 580.0f

    float startX = (winW - containerW) / 2.f;
    float startY = (winH - containerH) / 2.f + 30.f;

//    titleText.setPosition(startX, startY - 80.f);
//    subTitleText.setPosition(startX, startY - 40.f);


    // --- [THÊM] 1. UPDATE CONTROLLER & BINDING ---
    // Chỉ update nút khi không có card nào đang mở rộng (để tránh xung đột input)
    if (!expandedCard)
    {
        btnBack->update(window, dt);

        // [LOGIC BINDING] Lấy scale từ nút, áp dụng cho Text & Icon
        float s = btnBack->getScale();

        titleText.setScale(s, s);
        backIcon.setScale(s, s);

        // [LOGIC HOVER MÀU SẮC] Làm tối nhẹ khi hover (Apple Style)
        sf::Color targetColor = Theme::Color::TextPrimary;
        if (btnBack->isHovering()) {
            targetColor = sf::Color(80, 80, 80); // Màu xám đậm hơn chút
        }

        // Damp màu cho mượt
        sf::Color curTitle = titleText.getFillColor();
        sf::Color curIcon = backIcon.getFillColor();

        titleText.setFillColor(Utils::Math::Smoothing::dampColor(curTitle, targetColor, 0.1f, dt));
        backIcon.setFillColor(Utils::Math::Smoothing::dampColor(curIcon, targetColor, 0.1f, dt));
    }


    // --- 2. BACKGROUND COLOR LOGIC ---
    sf::Color targetWinColor = Theme::Color::Background;
    if(expandedCard) {
        targetWinColor = getPaleColor(expandedCard->getThemeColor());
    }
    currentWindowColor = Utils::Math::Smoothing::dampColor(currentWindowColor, targetWinColor, Theme::Animation::BgSmoothing, dt, Theme::Animation::BgMinSpeed);

//    std::cout << (int)currentWindowColor.r << " " << (int)currentWindowColor.g << " " << (int)currentWindowColor.b << " " << (int)currentWindowColor.a << "\n";

    // --- 3. CARD LOGIC (HERO ANIMATION) ---
    if(expandedCard != nullptr)
    {
        // MODE: EXPANDED
        for(auto c : cards)
        {
            if(c == expandedCard)
            {
                // Thẻ Hero chiếm trọn bảng 1200x580
//                c->setBackgroundVisible(true);
                // Target là TÂM của bảng
                float centerX = startX + containerW / 2.0f;
                float centerY = startY + containerH / 2.0f;
                c->setTarget({centerX, centerY}, {containerW, containerH});
                c->setExpanded(true);
                c->setOpacity(255.0f); // Đảm bảo rõ nét
            }
            else
            {
                // Các thẻ khác trượt ra ngoài
//                c->setGhostMode(true);
                float offX = (c->getId() < expandedCard->getId()) ? startX + (containerW / 8.f) - 300.f : startX + containerW + 300.f;
                c->setTarget({offX, startY + containerH / 2.f}, {containerW / 4.f, containerH});
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
            // Tính Tâm
            float centerX = currentX + w / 2.0f;
            float centerY = startY + containerH / 2.0f;

            cards[i]->setTarget({centerX, centerY}, {w, containerH});
            cards[i]->setSelected(i == selectedIndex);
            cards[i]->setExpanded(false);

            GUI::CardPos pos = GUI::CardPos::Middle;
            if(i == 0) pos = GUI::CardPos::First;
            else if(i == (int)cards.size() - 1) pos = GUI::CardPos::Last;

            cards[i]->setCardPosition(pos);

            currentX += w;
        }
    }

    // --- CUỐI HÀM: KIỂM TRA XEM CÓ AI ĐANG DI CHUYỂN KHÔNG ---
    isAnimating = (expandedCard != nullptr);
    for(auto c : cards)
    {
        if (!c->isSettled())
        {
            isAnimating = true;
            break; // Chỉ cần 1 ông đang chạy là tính cả hội đang chạy
        }
    }

    for(auto c : cards) c->update(dt, window);
}



void MenuState::handleInput(sf::Event& event)
{
    // [THÊM] Nếu đang chuyển cảnh thì không nhận input click
    if (isTransitioning) return;


    // THÊM ĐOẠN NÀY ĐỂ TẠO VIỀN ĐEN KHI CO GIÃN
    if (event.type == sf::Event::Resized)
    {
        Utils::System::applyLetterboxView(window, event.size.width, event.size.height);
    }

    if(event.type == sf::Event::Closed) window.close();



    if (!expandedCard) {
        btnBack->handleEvent(event, window);
    }

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

            states.push(new VisualizerState(window, states, new DS::Heap(true)));  ///**************************
        };

        auto onBack = [this]() {
            this->expandedCard = nullptr;//        updateInTransition(dt);

        };

        // Gán Callbacks (Bắt buộc để nút hoạt động)
        // Gán Callbacks (Sửa từ hàm set... thành gán biến)
        cards[i]->onSelect = onSelect;
        cards[i]->onViewMore = onViewMore;
        cards[i]->onStart = onStart;
        cards[i]->onBack = onBack;

        cards[i]->handleEvent(event, window);
    }
}

void MenuState::update(float dt)
{

    if (isTransitioning)
    {
        updateInTransition(dt);
    }
    else
    {
        updateLayout(dt); // Chỉ chạy layout Accordion khi đã vào xong
    }
}

void MenuState::draw()
{
    window.clear(currentWindowColor);
    window.pushGLStates();

//    assert(isTransitioning == false);


    // 1. VẼ BẢNG NỀN (Luôn vẽ)
    // Đây là cái bảng "nối" từ hiệu ứng Morph của CategoriesState sang
    window.draw(shadowSprite);
//    bgBoard.setFillColor(sf::Color::Cyan);
    window.draw(bgBoard);

    // 2. VẼ HEADER (Title, SubTitle, BackButton)
    if(!expandedCard)
    {
        // Vẽ nút Back trước (lớp nền tàng hình, dùng để debug hitbox nếu cần)
        btnBack->draw(window);

        // Vẽ Icon và Title đè lên trên
        window.draw(backIcon);
        window.draw(titleText);

        window.draw(subTitleText);
    }


    // Dùng Hằng số mới cho kích thước bảng vẽ
    float containerW = Theme::Style::AlgoBoardW; // 1200
    float containerH = Theme::Style::AlgoBoardH; // 580

    float startX = (Utils::System::DESIGN_WIDTH - containerW) / 2.f;
    float startY = (Utils::System::DESIGN_HEIGHT - containerH) / 2.f + 30.f;

//    if(!expandedCard)
//    {
//        window.draw(titleText);
//        window.draw(subTitleText);
//    }
//    if(isAnimating)
//    {
//        GUI::Squircle bgRect({containerW, containerH}, Theme::Style::AlgoRadius);
//        bgRect.setCurvature(Theme::Style::SquircleCurvature);
//        bgRect.setPosition(startX, startY);
//        bgRect.setFillColor(sf::Color::White);
//        window.draw(bgRect);
//    }

    // --- VẼ BẢNG NỀN ---
    // Bóng đổ (Dùng AlgoRadius 48.0f)
//    GUI::Squircle shadowRect({containerW, containerH}, Theme::Style::AlgoRadius);
//    shadowRect.setCurvature(Theme::Style::SquircleCurvature);
//    shadowRect.setPosition(startX + 10.f, startY + 10.f);
//    shadowRect.setFillColor(Theme::Color::Shadow);
//    window.draw(shadowRect);

    // Nền trắng
//    GUI::Squircle bgRect({containerW, containerH}, Theme::Style::AlgoRadius);
//    bgRect.setCurvature(Theme::Style::SquircleCurvature);
//    bgRect.setPosition(startX, startY);
//    bgRect.setFillColor(sf::Color::White);
//    window.draw(bgRect);

    // --- SCISSOR TEST ---
    sf::Vector2u windowSize = sf::Vector2u(window.getSize().x, window.getSize().y);
    sf::Vector2i scissorPos = window.mapCoordsToPixel({startX, startY + containerH});
    sf::Vector2i scissorSize = window.mapCoordsToPixel({startX + containerW, startY}) - scissorPos;

    // Dùng window.getSize().y (kích thước thật) để lật trục Y cho OpenGL
    int scissorY = window.getSize().y - scissorPos.y;

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

void MenuState::updateInTransition(float dt)
{
    transitionTimer += dt;


    // --- [THÊM] ANIMATION CHO ICON ---
    // Logic: Icon trượt từ vị trí (Text.x - 10) về (Text.x - 30)
    // Delay 0.1s để Text ổn định vị trí trước

    sf::Vector2f textPos = titleText.getPosition();
    // Vị trí đích của Icon (cách Text 30px về bên trái)
    float targetIconX = textPos.x - 30.0f;
    float targetIconY = textPos.y;

    if (transitionTimer > 0.01f)
    {
        // Tính tiến độ animation cho Icon (diễn ra trong 0.4s)
        float tIcon = std::min((transitionTimer - 0.01f) / 0.4f, 1.0f);
        float easeIcon = Utils::Math::Easing::easeOutCubic(tIcon);

        // Lerp từ (Target + 20px) về Target -> Hiệu ứng trượt sang trái
        float currentIconX = Utils::Math::Easing::lerp(targetIconX + 20.0f, targetIconX, easeIcon);

        backIcon.setPosition(currentIconX, targetIconY);

        // Fade In (Alpha 0 -> 255)
        sf::Color ic = backIcon.getFillColor();
        ic.a = (sf::Uint8)(easeIcon * 255.0f);
        backIcon.setFillColor(ic);
    }
    else
    {
        // Lúc đầu ẩn Icon đi
        sf::Color ic = backIcon.getFillColor();
        ic.a = 0;
        backIcon.setFillColor(ic);
    }



    // Thông số Layout chuẩn
    float containerW = Theme::Style::AlgoBoardW;
    float containerH = Theme::Style::AlgoBoardH;
    float startX = (Utils::System::DESIGN_WIDTH - containerW) / 2.0f;
    float startY = (Utils::System::DESIGN_HEIGHT - containerH) / 2.0f + 30.f;
    float cardW = containerW / cards.size();

    // 1. UPDATE CARDS (Staggered Animation)
    for (int i = 0; i < (int)cards.size(); ++i)
    {
        float delay = i * Theme::Animation::MenuStaggerDelay; // Mỗi thẻ trễ nhau 0.05s
        float duration = Theme::Animation::MenuSlideDuration;   // Thời gian mỗi thẻ chạy

        // Tính tiến độ t (0.0 -> 1.0)
        float t = (transitionTimer - delay) / duration;
        t = std::max(0.0f, std::min(t, 1.0f));

        if (t > 0.0f)
        {
            float ease = Utils::Math::Easing::easeOutQuart(t);

            // A. FADE IN (Hiện dần)
            float alpha = ease * 255.0f;
//            cards[i]->setOpacity(alpha);
            cards[i]->setOpacity(alpha);

            // B. SLIDE UP (Set lại target về vị trí chuẩn)
            // Chỉ cần set 1 lần hoặc set liên tục đều được vì MenuCard damp vị trí
            float finalCenterX = startX + i * cardW + cardW/2.0f;
            float finalCenterY = startY + containerH/2.0f;

            // Set Target về Y chuẩn (startY)
            cards[i]->setTarget({finalCenterX, finalCenterY}, {cardW, containerH});
        }
        else cards[i]->setOpacity(0.0f);

        // Update physics của card
        cards[i]->update(dt, window);
    }

//    float shadowT = std::min(transitionTimer / Theme::Animation::ShadowDuration, 1.0f);
//    float easeShadow = Utils::Math::Easing::easeInCubic(shadowT);

//    sf::Uint8 a = (sf::Uint8)(easeShadow * 255.0f);
//    shadowSprite.setColor(sf::Color(255, 255, 255, a));

//    std::cout << (int)a << "\n";

//    window.draw(shadowSprite);

    // 2. FADE IN SUBTITLE (Chậm hơn chút)
    float textT = std::min(transitionTimer / Theme::Animation::MenuSubtitleDuration, 1.0f); // 0.8s
    float easeText = Utils::Math::Easing::easeOutQuart(textT);

    sf::Color sCol = subTitleText.getFillColor();
    sCol.a = (sf::Uint8)(easeText * 255);
    subTitleText.setFillColor(sCol);

    // 3. KẾT THÚC
    // Tổng thời gian = (Delay thẻ cuối) + Duration
//    float totalTime = ((cards.size() - 1) * 0.05f) + 0.5f + 0.2f; // +0.2 dư ra cho chắc
    float totalTime = ((cards.size() - 1) * Theme::Animation::MenuStaggerDelay)
                      + Theme::Animation::MenuSlideDuration
                      + 0.1f; // +0.1f buffer an toàn

    if (transitionTimer > totalTime)
    {
        isTransitioning = false;
//        std::cout << transitionTimer << "\n";

        // Finalize state
        for(auto c : cards) {
//            c->setOpacity(255.0f);
            c->setOpacity(255.0f);
        }
        subTitleText.setFillColor(Theme::Color::TextSecondary);
    }
}
