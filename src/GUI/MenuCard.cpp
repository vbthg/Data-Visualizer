#include "GUI/MenuCard.h"
#include "ResourceManager.h"
#include "Smoothing.h"
#include <iostream>
#include <cmath>
#include <algorithm> // std::min


namespace GUI
{
    // Alias cho ngắn gọn
    namespace Theme = Utils::Graphics::Theme;

    MenuCard::MenuCard(int id, const std::string& title, const std::string& number, sf::Color themeColor)
        : id(id), themeColor(themeColor), selected(false), expanded(false)
    {
        auto& res = ResourceManager::getInstance();
        // Giả sử bạn load font từ Resource
        sf::Font& fontBold = res.getFont("assets/fonts/font.ttf");
        sf::Font& fontReg = res.getFont("assets/fonts/font.ttf");

        // --- 1. SETUP TEXT (Dùng màu từ Theme nếu cần) ---
        textNumber.setFont(fontBold);
        textNumber.setString(number);
        textNumber.setCharacterSize(30);
        textNumber.setFillColor(Theme::Color::TextSecondary); // Xám nhạt

        textTitle.setFont(fontBold);
        textTitle.setString(title);
        textTitle.setCharacterSize(22);
        textTitle.setFillColor(Theme::Color::TextPrimary); // Đen

        textBigTitle.setFont(fontBold);
        textBigTitle.setString(title);
        textBigTitle.setCharacterSize(80);
        textBigTitle.setFillColor(sf::Color::White);

        imgPlaceholder.setFillColor(sf::Color(255, 255, 255, 50));
        imgPlaceholder.setSize({150.f, 300.f});

        // --- 2. SETUP BUTTONS (Dùng Theme Radius) ---
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
        // Dùng Radius chuẩn của Button trong Theme (hoặc to hơn xíu vì nút này to)
        btnStart->setCornerRadius(30.f);

        // --- INIT ANIMATION STATE ---
        animTimer = 1.0f; // Đánh dấu là đã chạy xong

        // Khởi tạo vị trí mặc định
        currentPos = targetPos = startPos = {0, 0};
        currentSize = targetSize = startSize = {100, 100};

        // --- 4. CONFIG VISUALS ---
        contentAlpha = 0.0f;
        cornerRadius = Theme::Style::ItemRadius; // 24.0f
        cardPos = CardPos::Middle;

        currentBigTitlePos = {0, 0};
        currentImgPos = {0, 0};

        // Setup Squircle Shape
        bgShape.setCurvature(Theme::Style::SquircleCurvature); // 4.0f (Chuẩn Apple)
        bgShape.setFillColor(sf::Color(255, 255, 255, 0));
    }

    MenuCard::~MenuCard()
    {
        delete btnViewMore;
        delete btnBack;
        delete btnStart;
    }

    void MenuCard::setCardPosition(CardPos pos, float radius)
    {
        cardPos = pos;
        cornerRadius = radius;
    }

    void MenuCard::setTarget(const sf::Vector2f& pos, const sf::Vector2f& size)
    {
        float epsilon = 0.5f;
        bool posChanged = (std::abs(pos.x - targetPos.x) > epsilon || std::abs(pos.y - targetPos.y) > epsilon);
        bool sizeChanged = (std::abs(size.x - targetSize.x) > epsilon || std::abs(size.y - targetSize.y) > epsilon);

        if (posChanged || sizeChanged)
        {
            // 1. Chốt trạng thái hiện tại làm điểm xuất phát
            startPos = currentPos;
            startSize = currentSize;

            // 2. Đặt đích đến mới
            targetPos = pos;
            targetSize = size;

            // 3. Reset đồng hồ để bắt đầu chạy animation
            animTimer = 0.0f;
        }
    }

    void MenuCard::setSelected(bool sel)
    {
        selected = sel;
    }

    void MenuCard::setExpanded(bool exp)
    {
        expanded = exp;
    }

    void MenuCard::handleEvent(const sf::Event& event, const sf::RenderWindow& window,
                               std::function<void()> onSelect,
                               std::function<void()> onViewMore,
                               std::function<void()> onStart,
                               std::function<void()> onBack)
    {
        // Logic Handle Event giữ nguyên, chỉ thêm const vào tham số hàm
        if(expanded)
        {
            btnBack->handleEvent(event, window);
            btnBack->onClick = onBack;

            btnStart->handleEvent(event, window);
            btnStart->onClick = onStart;
            return;
        }

        if(selected)
        {
            btnViewMore->handleEvent(event, window);
            // Check click logic cho View More...
             if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                 if(btnViewMore->isHovering()) onViewMore();
            }
        }

        if(!expanded && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if(bgShape.getGlobalBounds().contains(mPos))
            {
                if(!selected) onSelect();
            }
        }
    }

    void MenuCard::update(float dt, sf::RenderWindow& window)
    {
        // 1. Tăng đồng hồ
        animTimer += dt;

        // 2. Tính tỉ lệ t (từ 0.0 đến 1.0)
        // Dùng duration từ Theme (ví dụ 0.8s)
        float t = std::min(animTimer / Theme::Animation::CardAnimDuration, 1.0f);

        // 3. Áp dụng Ease Out Cubic: 1 - (1-t)^3
        float easeT = Utils::Math::Easing::easeOutCubic(t);

        // 4. Nội suy (Lerp)
        currentPos.x = Utils::Math::Easing::lerp(startPos.x, targetPos.x, easeT);
        currentPos.y = Utils::Math::Easing::lerp(startPos.y, targetPos.y, easeT);

        currentSize.x = Utils::Math::Easing::lerp(startSize.x, targetSize.x, easeT);
        currentSize.y = Utils::Math::Easing::lerp(startSize.y, targetSize.y, easeT);

        // --- 2. UPDATE SHAPE ---
        bgShape.setPosition(currentPos);
        bgShape.setSize(currentSize);

        // Logic bo góc (Smart Corner Radius)
        if(expanded)
        {
            // Khi mở rộng: Bo tròn đều 4 góc to hơn (CardRadius = 48.f)
            float r = Theme::Style::CardRadius;
            bgShape.setCornerRadius(r, r, r, r);
        }
        else
        {
            // Khi thu gọn: Bo theo vị trí Accordion (ItemRadius = 24.f)
            switch(cardPos)
            {
                case CardPos::First:
                    bgShape.setCornerRadius(cornerRadius, 0, 0, cornerRadius);
                    break;
                case CardPos::Last:
                    bgShape.setCornerRadius(0, cornerRadius, cornerRadius, 0);
                    break;
                case CardPos::Middle:
                    bgShape.setCornerRadius(0, 0, 0, 0);
                    break;
                case CardPos::Single:
                    bgShape.setCornerRadius(cornerRadius, cornerRadius, cornerRadius, cornerRadius);
                    break;
            }
        }

        // --- 3. COLOR & ALPHA (SMOOTHING) ---
        // Dùng hằng số từ Theme
        sf::Color targetColor = (selected || expanded) ? themeColor : sf::Color(255, 255, 255, 0);

        bgShape.setFillColor(Utils::Math::Smoothing::dampColor(
            bgShape.getFillColor(),
            targetColor,
            Theme::Animation::ColorSmoothing, // 0.1f
            dt,
            Theme::Animation::ColorSnapSpeed  // 500.0f
        ));

        float desiredAlpha = (selected || expanded) ? 255.0f : 0.0f;
        contentAlpha = Utils::Math::Smoothing::damp(contentAlpha, desiredAlpha, 0.05f, dt, 500.0f);

        // --- 4. CONTENT ANIMATION (SWIPE) ---
        float midX = currentPos.x + currentSize.x / 2.0f;

        if(expanded)
        {
            // Các vị trí hardcode này bạn có thể đưa vào Theme nốt nếu muốn kỹ tính
            sf::Vector2f targetBigTitlePos = {currentPos.x + 80.f, currentPos.y + 100.f};
            sf::Vector2f targetImgPos = {midX - imgPlaceholder.getSize().x/2, currentPos.y + 250.f};

            // Dùng damp cho nội dung bên trong (để nó lướt theo sau background 1 nhịp)
            currentBigTitlePos.x = Utils::Math::Smoothing::damp(currentBigTitlePos.x, targetBigTitlePos.x, 0.08f, dt);
            currentBigTitlePos.y = Utils::Math::Smoothing::damp(currentBigTitlePos.y, targetBigTitlePos.y, 0.08f, dt);

            currentImgPos.x = Utils::Math::Smoothing::damp(currentImgPos.x, targetImgPos.x, 0.1f, dt);
            currentImgPos.y = Utils::Math::Smoothing::damp(currentImgPos.y, targetImgPos.y, 0.1f, dt);

            textBigTitle.setPosition(currentBigTitlePos);
            imgPlaceholder.setPosition(currentImgPos);

            // Update Buttons
            btnBack->setPosition({currentPos.x + 50.f, currentPos.y + 50.f});
            btnBack->update(window, dt);

            btnStart->setPosition({midX - 110.f, currentPos.y + 600.f});
            btnStart->update(window, dt);
        }
        else
        {
            // Accordion Elements
            currentBigTitlePos = {currentPos.x + 200.f, currentPos.y + 100.f};
            currentImgPos = {midX, currentPos.y + 300.f};

            // Text & Number
            sf::Color numColor = (selected) ? sf::Color(255, 255, 255, 150) : themeColor;
            textNumber.setFillColor(Utils::Math::Smoothing::dampColor(textNumber.getFillColor(), numColor, 0.1f, dt));

            // Căn giữa số
            sf::FloatRect nb = textNumber.getLocalBounds();
            textNumber.setOrigin(nb.left + nb.width/2.f, nb.top + nb.height/2.f);
            textNumber.setPosition(midX, currentPos.y + 40.f + nb.height/2.f);

            sf::Color titleColor = (selected) ? sf::Color::White : Theme::Color::TextPrimary;
            textTitle.setFillColor(Utils::Math::Smoothing::dampColor(textTitle.getFillColor(), titleColor, 0.1f, dt));

            // Căn giữa title
            sf::FloatRect tb = textTitle.getLocalBounds();
            textTitle.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
            textTitle.setPosition(midX, currentPos.y + 90.f + tb.height/2.f);

            if(contentAlpha > 10.f)
            {
                btnViewMore->setPosition({midX - 50.f, currentPos.y + 180.f});
                btnViewMore->update(window, dt);
            }
        }
    }

    void MenuCard::draw(sf::RenderWindow& window)
    {
        window.draw(bgShape);

        if(expanded)
        {
            window.draw(textBigTitle);
            window.draw(imgPlaceholder);
            btnBack->draw(window);
            btnStart->draw(window);
        }
        else
        {
            window.draw(textNumber);
            window.draw(textTitle);
            if(contentAlpha > 10.f)
            {
                btnViewMore->draw(window);
            }
        }
    }

} // namespace GUI
