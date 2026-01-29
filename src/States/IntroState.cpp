#include "IntroState.h"
#include "MenuState.h" // Để chuyển cảnh sang Menu
#include "Easing.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// --- IMPLEMENTATION: AppleText ---
void AppleText::setup(const sf::Font& font, const std::string& content, unsigned int size, sf::Vector2f centerPos)
{
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color(0, 0, 0, 0)); // Ẩn ban đầu

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(centerPos);

    alpha = 0.0f;
    targetAlpha = 0.0f;
    offsetY = 30.0f;
    targetOffsetY = 30.0f;
    isVisible = false;
}

void AppleText::show()
{
    isVisible = true;
    targetAlpha = 255.0f;
    targetOffsetY = 0.0f;
}

void AppleText::update(float dt)
{
    if(!isVisible) return;

    alpha = Utils::Math::Smoothing::damp(alpha, targetAlpha, 0.02f, dt);
    offsetY = Utils::Math::Smoothing::damp(offsetY, targetOffsetY, 0.02f, dt);

    sf::Color c = text.getFillColor();
    c.a = (sf::Uint8)alpha;
    text.setFillColor(c);
}

void AppleText::draw(sf::RenderWindow& window)
{
    if(alpha < 1.0f) return;

    sf::Vector2f oldPos = text.getPosition();
    text.setPosition(oldPos.x, oldPos.y + offsetY);
    window.draw(text);
    text.setPosition(oldPos);
}

// --- IMPLEMENTATION: IntroNode ---
IntroNode::IntroNode(int id, const sf::Texture& tex, sf::Vector2f startPos)
    : id(id)
{
    sprite.setTexture(tex);
    sf::Vector2u size = tex.getSize();
    sprite.setOrigin(size.x / 2.0f, size.y / 2.0f);
    sprite.setScale(0.07f, 0.07f); // Chỉnh scale tùy kích thước ảnh của bạn
    sprite.setColor(sf::Color(0, 122, 255)); // Apple Blue

    currentPos = startPos;
    targetPos = startPos;
    parent = nullptr;
}

void IntroNode::update(float dt)
{
    currentPos.x = Utils::Math::Smoothing::damp(currentPos.x, targetPos.x, 0.1f, dt);
    currentPos.y = Utils::Math::Smoothing::damp(currentPos.y, targetPos.y, 0.1f, dt);
    sprite.setPosition(currentPos);
}

// --- IMPLEMENTATION: IntroState ---

IntroState::IntroState(sf::RenderWindow& window, std::stack<State*>& states)
    : window(window), states(states)
{
    btnStart = nullptr;
    init();
}

IntroState::~IntroState()
{
    if(btnStart) delete btnStart;
}

void IntroState::init()
{
    // Lấy Resource qua Singleton
    sf::Font& fontBold = ResourceManager::getInstance().getFont("assets/fonts/font.ttf");
    sf::Font& fontReg = ResourceManager::getInstance().getFont("assets/fonts/font.ttf");
    sf::Texture& nodeTex = ResourceManager::getInstance().getTexture("assets/textures/node.png");

    view = window.getDefaultView();
    currentCamCenter = view.getCenter();
    targetCamCenter = currentCamCenter;
    currentZoom = 1.0f;
    targetZoom = 1.0f;
    isFinished = false;

    // Setup 9 Nodes (8 node cây ban đầu + 1 node rơi xuống)
    nodes.clear();
    for(int i = 0; i < 9; ++i)
    {
        // Node thứ 9 (index 8) ở trên trời
        sf::Vector2f startPos = (i == 8) ? sf::Vector2f(0, -500) : sf::Vector2f(0, 0);
        nodes.emplace_back(i, nodeTex, startPos);
    }

    // Setup Text
    sf::Vector2f center = { (float)window.getSize().x / 2.f, (float)window.getSize().y / 2.f };
    textBalance.setup(fontBold, "Balance matters.", 60, center - sf::Vector2f(0, 200));
    textDynamic.setup(fontReg, "Structures are not static.", 40, center + sf::Vector2f(0, 250));
    textRespond.setup(fontBold, "They respond.", 50, center + sf::Vector2f(0, 310));

    // Setup Button
    btnStart = new GUI::Button(fontReg, "Start Exploring", {220.f, 50.f});
    btnStart->setPosition({center.x - 110.f, center.y + 180.f});
    btnStart->setCornerRadius(25.f);
    btnStart->applyPreset(GUI::ButtonPreset::Clean);
    btnStart->setOutline(1.f, sf::Color(200, 200, 200));
    btnAlpha = 0.0f;

    // Logic nút Start: Vào Menu
    btnStart->onClick = [this]() {
        return;
//        this->states.push(new MenuState(this->window, this->states));
    };

    // --- SETUP TRANSITION (MỚI) ---
    isTransitioning = false;
    transitionTimer = 0.0f;
    transitionAlpha = 0.0f;

    overlayRect.setSize(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
    overlayRect.setFillColor(sf::Color(255, 255, 255, 0)); // Trắng trong suốt

    // Logic nút Start: Kích hoạt hiệu ứng
    btnStart->onClick = [this]() {
        // Nếu chưa transition thì bắt đầu
        if (!this->isTransitioning)
        {
            this->isTransitioning = true;
            this->transitionTimer = 0.0f;
        }
    };

    setupKeyframes();

    currentStepIndex = -1;
    stepTimer = 0;
    nextStep();
}

void IntroState::setupKeyframes()
{
    sf::Vector2f center = { (float)window.getSize().x / 2.f, (float)window.getSize().y / 2.f - 100 };

    // --- STEP 0: INITIAL (Cây lệch) ---
    AnimationStep step0;
    step0.duration = 2.0f;
    step0.camFocus = center + sf::Vector2f(100, 200);
    step0.camZoom = 0.8f;
    step0.nodePositions = {
        center, center + sf::Vector2f(60, 80), center + sf::Vector2f(120, 160),
        center + sf::Vector2f(60, 240), center + sf::Vector2f(0, 320),
        center + sf::Vector2f(-60, 400), center + sf::Vector2f(0, 480), center + sf::Vector2f(60, 560),
        sf::Vector2f(0, -500) // Node 8
    };
    step0.parentIndices = {-1, 0, 1, 2, 3, 4, 5, 6, -1};
    steps.push_back(step0);

    // --- STEP 1: ROTATION 1 ---
    AnimationStep step1 = step0;
    step1.duration = 1.0f;
    step1.camFocus = center + sf::Vector2f(0, 400);
    step1.camZoom = 0.6f;
    step1.nodePositions[4] = center + sf::Vector2f(30, 320);
    step1.nodePositions[5] = center + sf::Vector2f(-30, 400);
    step1.nodePositions[6] = center + sf::Vector2f(90, 400);
    step1.parentIndices = {-1, 0, 1, 2, 3, 4, 4, 6, -1};
    steps.push_back(step1);

    // --- STEP 2: ROTATION 2 ---
    AnimationStep step2 = step1;
    step2.duration = 1.0f;
    step2.camFocus = center + sf::Vector2f(80, 200);
    step2.camZoom = 0.7f;
    step2.nodePositions[2] = center + sf::Vector2f(90, 160);
    step2.nodePositions[3] = center + sf::Vector2f(30, 240);
    step2.nodePositions[4] = center + sf::Vector2f(150, 240);
    steps.push_back(step2);

    // --- STEP 3: BALANCED (Hiện "Balance matters") ---
    AnimationStep step3;
    step3.duration = 3.0f;
    step3.camFocus = center + sf::Vector2f(0, 150);
    step3.camZoom = 1.0f;
    step3.nodePositions = {
        center, center + sf::Vector2f(-120, 100), center + sf::Vector2f(120, 100),
        center + sf::Vector2f(-180, 200), center + sf::Vector2f(-60, 200),
        center + sf::Vector2f(60, 200), center + sf::Vector2f(180, 200), center + sf::Vector2f(-220, 300),
        sf::Vector2f(0, -500)
    };
    step3.parentIndices = {-1, 0, 0, 1, 1, 2, 2, 3, -1};
    steps.push_back(step3);

    // --- STEP 4: INSERT NODE (Hiện "Not static") ---
    AnimationStep step4 = step3;
    step4.duration = 2.0f;
    step4.camFocus = center + sf::Vector2f(180, 200); // Nhìn vào nơi node rơi
    step4.camZoom = 0.8f;
    step4.nodePositions[8] = center + sf::Vector2f(240, 300); // Node 8 rơi xuống con phải của 6
    step4.parentIndices[8] = 6;
    steps.push_back(step4);

    // --- STEP 5: REBALANCE (Hiện "They respond") ---
    AnimationStep step5 = step4;
    step5.duration = 4.0f;
    step5.camFocus = center + sf::Vector2f(0, 150);
    step5.camZoom = 0.9f;

    // Xoay: Node 6 lên thay Node 2
    step5.nodePositions[6] = step3.nodePositions[2];
    step5.nodePositions[2] = step3.nodePositions[2] + sf::Vector2f(-60, 100);
    step5.nodePositions[8] = step3.nodePositions[2] + sf::Vector2f(60, 100);

    step5.parentIndices[6] = 0; // 6 con Root
    step5.parentIndices[2] = 6; // 2 con 6
    step5.parentIndices[8] = 6; // 8 con 6
    steps.push_back(step5);
}

void IntroState::nextStep()
{
    currentStepIndex++;
    if(currentStepIndex >= steps.size()) return;

    const auto& step = steps[currentStepIndex];

    for(size_t i = 0; i < nodes.size(); ++i)
    {
        if(i < step.nodePositions.size())
        {
            nodes[i].targetPos = step.nodePositions[i];
            int pIdx = step.parentIndices[i];
            nodes[i].parent = (pIdx != -1) ? &nodes[pIdx] : nullptr;
        }
    }

    targetCamCenter = step.camFocus;
    targetZoom = step.camZoom;
    stepTimer = step.duration;
}

void IntroState::skipIntro()
{
    if(isFinished) return;

    // Tua ngay đến Step cuối
    currentStepIndex = 5;
    stepTimer = 0;

    // Apply vị trí ngay lập tức
    const auto& step = steps[currentStepIndex];
    for(size_t i = 0; i < nodes.size(); ++i)
    {
        if(i < step.nodePositions.size())
        {
            nodes[i].currentPos = step.nodePositions[i];
            nodes[i].targetPos = step.nodePositions[i];
            int pIdx = step.parentIndices[i];
            nodes[i].parent = (pIdx != -1) ? &nodes[pIdx] : nullptr;
        }
    }

    targetCamCenter = step.camFocus;
    targetZoom = step.camZoom;
    currentCamCenter = targetCamCenter;
    currentZoom = targetZoom;

    // Ẩn/Hiện Text
    textBalance.isVisible = false;
    textDynamic.show(); textDynamic.alpha = 255; textDynamic.offsetY = 0;
    textRespond.show(); textRespond.alpha = 255; textRespond.offsetY = 0;

    // Hiện nút Start
    btnAlpha = 255.0f;
    isFinished = true;
}

void IntroState::handleInput(sf::Event& event)
{
    // CHẶN INPUT KHI ĐANG TRANSITION
    if(isTransitioning) return;

    if(isFinished)
    {
        btnStart->handleEvent(event, window);
    }
    else
    {
        if(event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed)
        {
            skipIntro();
        }
    }
}

void IntroState::update(float dt)
{
    // 1. LOGIC TRANSITION (Flash to Clarity)
    if(isTransitioning)
    {
        transitionTimer += dt;
        float duration = 0.5f; // Nhanh, dứt khoát
        float t = std::min(transitionTimer / duration, 1.0f);

        // Easing cực mạnh (EaseInCubic) -> Hút vào
        float easedT = Utils::Math::Easing::easeInCubic(t);

        // Alpha: 0 -> 255 (Màn trắng hiện ra)
        transitionAlpha = easedT * 255.0f;

        // Zoom: 1.0 -> 0.4 (Lao sâu vào tâm cây)
        // Lerp từ zoom hiện tại xuống 0.4
        // (Lưu ý: Camera Zoom số càng nhỏ = Phóng càng to)
        targetZoom = 0.9f - (easedT * 0.6f);

        // Cập nhật màu overlay
        overlayRect.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)transitionAlpha));

        if(t >= 1.0f)
        {
            // Màn hình đã trắng xóa -> Chuyển cảnh
            states.push(new MenuState(window, states));

            // Reset transition (nếu sau này có pop state quay lại)
            isTransitioning = false;
            transitionTimer = 0.0f;
            return; // Dừng update frame này
        }
    }

    // 2. Logic Kịch bản Intro (Chỉ chạy khi chưa transition)
    if(!isFinished && !isTransitioning)
    {
        stepTimer -= dt;
        if(stepTimer <= 0 && currentStepIndex < steps.size() - 1)
        {
            nextStep();
        }
        else if(stepTimer <= 0 && currentStepIndex == steps.size() - 1)
        {
            isFinished = true;
        }

        // Logic Kịch bản Text (GIỮ NGUYÊN)
        if(currentStepIndex == 3) { if(!textBalance.isVisible) textBalance.show(); }
        else if(currentStepIndex == 4) { textBalance.targetAlpha = 0; if(stepTimer < 1.5f && !textDynamic.isVisible) textDynamic.show(); }
        else if(currentStepIndex == 5) { if(stepTimer < 3.5f && !textRespond.isVisible) textRespond.show(); }
    }

    // 3. Nút Start Fade in
    if(isFinished)
    {
        btnAlpha = Utils::Math::Smoothing::damp(btnAlpha, 255.0f, 0.05f, dt);
        btnStart->update(window, dt);
    }

    // 4. Update Cursor
    // (Nếu đang transition thì ẩn chuột hoặc để arrow cho đỡ rối)
    CursorType cursor = CursorType::Arrow;
    if(!isTransitioning && isFinished && btnStart->isHovering()) cursor = CursorType::Hand;
    window.setMouseCursor(ResourceManager::getInstance().getCursor(cursor));

    // 5. Update Objects & Texts
    for(auto& node : nodes) node.update(dt);
    textBalance.update(dt);
    textDynamic.update(dt);
    textRespond.update(dt);

    // 6. Smooth Camera
    // Nếu đang Transition, targetZoom đã được set ở trên, damp sẽ đi theo nó
    currentZoom = Utils::Math::Smoothing::damp(currentZoom, targetZoom, 0.05f, dt);
    currentCamCenter.x = Utils::Math::Smoothing::damp(currentCamCenter.x, targetCamCenter.x, 0.08f, dt);
    currentCamCenter.y = Utils::Math::Smoothing::damp(currentCamCenter.y, targetCamCenter.y, 0.08f, dt);

    view.setSize((float)window.getSize().x * currentZoom, (float)window.getSize().y * currentZoom);
    view.setCenter(currentCamCenter);
}

void IntroState::draw()
{
    window.setView(view);

    // Vẽ Dây
    for(const auto& node : nodes)
    {
        if(node.parent)
        {
            sf::Vertex line[] = {
                sf::Vertex(node.currentPos, sf::Color(150, 150, 150)),
                sf::Vertex(node.parent->currentPos, sf::Color(150, 150, 150))
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    // Vẽ Node
    for(auto& node : nodes)
    {
        window.draw(node.sprite);
    }

    // Vẽ UI Overlay
    window.setView(window.getDefaultView());

    // Mờ dần nền khi vào pha tương tác
    if(isFinished || currentStepIndex >= 4)
    {
        sf::RectangleShape overlay(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
        float targetOverlayAlpha = (currentStepIndex >= 4) ? 100.0f : 0.0f;
        if(isFinished) targetOverlayAlpha = 180.0f;

        overlay.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)targetOverlayAlpha));
        window.draw(overlay);
    }

    textBalance.draw(window);
    textDynamic.draw(window);
    textRespond.draw(window);

    // Vẽ Nút Start (Hack alpha bằng cách vẽ đè nếu nút chưa hỗ trợ setAlpha)
    if(btnAlpha > 1.0f)
    {
        // Nếu class Button của bạn chưa có setOpacity, nó sẽ hiện full.
        // Tạm thời vẽ như bình thường
        btnStart->draw(window);
    }

    // 3. VẼ TRANSITION (Cuối cùng - đè lên tất cả)
    if(isTransitioning)
    {
        // Đảm bảo vẽ ở Default View để full màn hình
        window.setView(window.getDefaultView());
        window.draw(overlayRect);
    }
}
