#include "MenuCard.h"
#include "ResourceManager.h"
#include "Smoothing.h"
#include "Easing.h" // Đảm bảo đã include file Easing
#include <iostream>
#include <cmath>
#include <algorithm>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    MenuCard::MenuCard(int id, const std::string& title, const std::string& number, sf::Color themeColor)
        : id(id), themeColor(themeColor), selected(false), expanded(false)
    {
        auto& res = ResourceManager::getInstance();
        sf::Font& fontBold = res.getFont("assets/fonts/font.ttf");
        sf::Font& fontReg = res.getFont("assets/fonts/font.ttf");

        // Setup Text
        textNumber.setFont(fontBold); textNumber.setString(number);
        textNumber.setCharacterSize(30); textNumber.setFillColor(Theme::Color::TextSecondary);

        textTitle.setFont(fontBold); textTitle.setString(title);
        textTitle.setCharacterSize(22); textTitle.setFillColor(Theme::Color::TextPrimary);

        textBigTitle.setFont(fontBold); textBigTitle.setString(title);
        textBigTitle.setCharacterSize(80); textBigTitle.setFillColor(sf::Color::White);

        imgPlaceholder.setFillColor(sf::Color(255, 255, 255, 50));
        imgPlaceholder.setSize({150.f, 300.f});

        // Setup Buttons
        btnViewMore = new Button(fontReg, "View More", {100.f, 36.f});
        btnViewMore->applyPreset(ButtonPreset::Ghost);
        btnViewMore->setCornerRadius(18.f);
        btnViewMore->setOutline(1.f, sf::Color::White);
        btnViewMore->setTextColor(sf::Color::White);

        btnBack = new Button(fontBold, "Back", {80.f, 40.f});
        btnBack->applyPreset(ButtonPreset::Ghost);
        btnBack->setTextColor(sf::Color::White);
        btnBack->setOutline(1.f, sf::Color::White);
        btnBack->setCornerRadius(20.f);

        btnStart = new Button(fontBold, "Start Visualization", {220.f, 60.f});
        btnStart->applyPreset(ButtonPreset::Clean);
        btnStart->setCornerRadius(30.f);

        // Init State
        animTimer = 1.0f;
        currentPos = targetPos = startPos = {0, 0};
        currentSize = targetSize = startSize = {100, 100};

        contentAlpha = 0.0f;
        cornerRadius = Theme::Style::ItemRadius;
        cardPos = CardPos::Middle;

        currentBigTitlePos = {0, 0};
        currentImgPos = {0, 0};

        // BẢN CŨ: Mặc định trong suốt
        bgShape.setCurvature(Theme::Style::SquircleCurvature);
        bgShape.setFillColor(sf::Color(255, 255, 255, 0));

        // Init Callbacks
        actionSelect = nullptr;
        actionViewMore = nullptr;
        actionStart = nullptr;
        actionBack = nullptr;
    }

    MenuCard::~MenuCard()
    {
        delete btnViewMore; delete btnBack; delete btnStart;
    }

    // --- SETTERS IMPLEMENTATION ---
    void MenuCard::setOnSelect(std::function<void()> callback) { actionSelect = callback; }
    void MenuCard::setOnViewMore(std::function<void()> callback) { actionViewMore = callback; }
    void MenuCard::setOnStart(std::function<void()> callback) { actionStart = callback; }
    void MenuCard::setOnBack(std::function<void()> callback) { actionBack = callback; }

    void MenuCard::setCardPosition(CardPos pos, float radius) { cardPos = pos; cornerRadius = radius; }

    void MenuCard::setTarget(const sf::Vector2f& pos, const sf::Vector2f& size)
    {
        float epsilon = 0.5f;
        bool posChanged = (std::abs(pos.x - targetPos.x) > epsilon || std::abs(pos.y - targetPos.y) > epsilon);
        bool sizeChanged = (std::abs(size.x - targetSize.x) > epsilon || std::abs(size.y - targetSize.y) > epsilon);

        if (posChanged || sizeChanged) {
            startPos = currentPos; startSize = currentSize;
            targetPos = pos; targetSize = size;
            animTimer = 0.0f;
        }
    }

    void MenuCard::setSelected(bool sel) { selected = sel; }
    void MenuCard::setExpanded(bool exp) { expanded = exp; }

    sf::FloatRect MenuCard::getGlobalBounds() const { return bgShape.getGlobalBounds(); }

    // --- HANDLE EVENT ---
    void MenuCard::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if(expanded)
        {
            btnBack->handleEvent(event, window);
            if(actionBack) btnBack->onClick = actionBack;

            btnStart->handleEvent(event, window);
            if(actionStart) btnStart->onClick = actionStart;
            return;
        }

        if(selected)
        {
            btnViewMore->handleEvent(event, window);
            if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                if(btnViewMore->isHovering() && actionViewMore) actionViewMore();
            }
        }

        if(!expanded && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if(bgShape.getGlobalBounds().contains(mPos))
            {
                if(!selected && actionSelect) actionSelect();
            }
        }
    }

    // --- UPDATE (CÓ LOGIC BO GÓC MỚI) ---
    void MenuCard::update(float dt, sf::RenderWindow& window)
    {
        // 1. Animation Di chuyển & Kích thước
        animTimer += dt;
        float t = std::min(animTimer / Theme::Animation::CardAnimDuration, 1.0f);
        float easeT = Utils::Math::Easing::easeOutCubic(t);

        currentPos.x = Utils::Math::Easing::lerp(startPos.x, targetPos.x, easeT);
        currentPos.y = Utils::Math::Easing::lerp(startPos.y, targetPos.y, easeT);
        currentSize.x = Utils::Math::Easing::lerp(startSize.x, targetSize.x, easeT);
        currentSize.y = Utils::Math::Easing::lerp(startSize.y, targetSize.y, easeT);

        bgShape.setPosition(currentPos);
        bgShape.setSize(currentSize);

        // 2. LOGIC BO GÓC THÔNG MINH (Smart Corner Radius Interpolation)
        // Xác định bán kính cơ bản (khi chưa mở rộng) cho 4 góc
        float baseR = cornerRadius;
        float rTL = 0, rTR = 0, rBR = 0, rBL = 0;

        switch(cardPos) {
            case CardPos::First:  rTL = baseR; rBL = baseR; break; // Bo trái
            case CardPos::Last:   rTR = baseR; rBR = baseR; break; // Bo phải
            case CardPos::Single: rTL = rTR = rBR = rBL = baseR; break; // Bo hết
            case CardPos::Middle: break; // Vuông
        }

        if (expanded)
        {
            // Đích đến: Bo tròn toàn bộ theo chuẩn của Bảng (CardRadius)
            float targetR = Theme::Style::CardRadius;

            // Tính toán tiến độ bo góc (Radius Progress)
            // * 2.0f: Tăng tốc gấp đôi so với di chuyển -> Bo xong khi mới đi được 50% quãng đường
            // Clamp ở 1.0f để không bị vượt quá
            float r_t = std::min(t * 2.0f, 1.0f);
            float r_ease = Utils::Math::Easing::easeOutCubic(r_t);

            // Nội suy từng góc: Từ giá trị ban đầu -> Giá trị đích
            // Nếu rTL ban đầu đã bằng targetR (ví dụ thẻ First), lerp sẽ trả về giữ nguyên -> Đúng yêu cầu.
            float curTL = Utils::Math::Easing::lerp(rTL, targetR, r_ease);
            float curTR = Utils::Math::Easing::lerp(rTR, targetR, r_ease);
            float curBR = Utils::Math::Easing::lerp(rBR, targetR, r_ease);
            float curBL = Utils::Math::Easing::lerp(rBL, targetR, r_ease);

            bgShape.setCornerRadius(curTL, curTR, curBR, curBL);
        }
        else
        {
            // Trạng thái bình thường: Giữ nguyên hình dáng ban đầu
            bgShape.setCornerRadius(rTL, rTR, rBR, rBL);
        }

        // 3. Logic Màu & Alpha (Giữ nguyên)
        sf::Color targetColor = (selected || expanded) ? themeColor : sf::Color(255, 255, 255, 0);
        bgShape.setFillColor(Utils::Math::Smoothing::dampColor(bgShape.getFillColor(), targetColor, Theme::Animation::ColorSmoothing, dt, Theme::Animation::ColorSnapSpeed));

        float desiredAlpha = (selected || expanded) ? 255.0f : 0.0f;
        contentAlpha = Utils::Math::Smoothing::damp(contentAlpha, desiredAlpha, 0.05f, dt, 500.0f);

        float midX = currentPos.x + currentSize.x / 2.0f;

        if(expanded) {
            sf::Vector2f targetBigTitlePos = {currentPos.x + 80.f, currentPos.y + 100.f};
            sf::Vector2f targetImgPos = {midX - imgPlaceholder.getSize().x/2, currentPos.y + 250.f};

            currentBigTitlePos.x = Utils::Math::Smoothing::damp(currentBigTitlePos.x, targetBigTitlePos.x, 0.08f, dt);
            currentBigTitlePos.y = Utils::Math::Smoothing::damp(currentBigTitlePos.y, targetBigTitlePos.y, 0.08f, dt);
            currentImgPos.x = Utils::Math::Smoothing::damp(currentImgPos.x, targetImgPos.x, 0.1f, dt);
            currentImgPos.y = Utils::Math::Smoothing::damp(currentImgPos.y, targetImgPos.y, 0.1f, dt);

            textBigTitle.setPosition(currentBigTitlePos);
            imgPlaceholder.setPosition(currentImgPos);

            btnBack->setPosition({currentPos.x + 50.f, currentPos.y + 50.f});
            btnBack->update(window, dt);
            btnStart->setPosition({midX - 110.f, currentPos.y + 600.f});
            btnStart->update(window, dt);
        } else {
            currentBigTitlePos = {currentPos.x + 200.f, currentPos.y + 100.f};
            currentImgPos = {midX, currentPos.y + 300.f};

            sf::Color numColor = (selected) ? sf::Color(255, 255, 255, 150) : themeColor;
            textNumber.setFillColor(Utils::Math::Smoothing::dampColor(textNumber.getFillColor(), numColor, 0.1f, dt));

            sf::FloatRect nb = textNumber.getLocalBounds();
            textNumber.setOrigin(nb.left + nb.width/2.f, nb.top + nb.height/2.f);
            textNumber.setPosition(midX, currentPos.y + 40.f + nb.height/2.f);

            sf::Color titleColor = (selected) ? sf::Color::White : Theme::Color::TextPrimary;
            textTitle.setFillColor(Utils::Math::Smoothing::dampColor(textTitle.getFillColor(), titleColor, 0.1f, dt));

            sf::FloatRect tb = textTitle.getLocalBounds();
            textTitle.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
            textTitle.setPosition(midX, currentPos.y + 90.f + tb.height/2.f);

            if(contentAlpha > 10.f) {
                btnViewMore->setPosition({midX - 50.f, currentPos.y + 180.f});
                btnViewMore->update(window, dt);
            }
        }
    }

    void MenuCard::draw(sf::RenderWindow& window)
    {
        window.draw(bgShape);
        if(expanded) {
            window.draw(textBigTitle);
            window.draw(imgPlaceholder);
            btnBack->draw(window);
            btnStart->draw(window);
        } else {
            window.draw(textNumber);
            window.draw(textTitle);
            if(contentAlpha > 10.f) btnViewMore->draw(window);
        }
    }
}
