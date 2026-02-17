#include "MenuCard.h"
#include "ResourceManager.h"
#include "Smoothing.h"
#include "Easing.h" // Đảm bảo đã include file Easing
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    sf::Color applyAlpha(const sf::Color& color, float alphaMult) {
        return sf::Color(color.r, color.g, color.b, (sf::Uint8)(color.a * alphaMult));
    }

    MenuCard::MenuCard(const CardConfig& conf)
        : config(conf), selected(false), expanded(false), cardPos(CardPos::Single), globalAlpha(255.0f)
    {
        auto& res = ResourceManager::getInstance();
        namespace Theme = Utils::Graphics::Theme;

        // 1. Init Kích thước & Vị trí
        currentSize = targetSize = config.initialSize;
        currentPos = targetPos = {0.0f, 0.0f};

        // 2. Setup Background
        bgShape.setSize(currentSize);
        bgShape.setCurvature(Theme::Style::SquircleCurvature);
        bgShape.setOrigin(currentSize.x / 2.f, currentSize.y / 2.f);
        bgShape.setFillColor(Theme::Color::Background);
        bgShape.setRadius(0.f);

        // 3. Setup Icon
        if (config.iconTexture) {
            config.iconTexture->setSmooth(true);
            iconSprite.setTexture(*config.iconTexture);
            sf::FloatRect iconBounds = iconSprite.getLocalBounds();
            iconSprite.setOrigin(iconBounds.width / 2.f, iconBounds.height / 2.f);
            iconSprite.setScale(0.f, 0.f);
        }

        // 4. Setup Texts
        sf::Font& fontBold = res.getFont("assets/fonts/font.ttf");

        textNumber.setFont(fontBold);
        textNumber.setString(config.number);
        textNumber.setCharacterSize(30);
        textNumber.setFillColor(Theme::Color::TextSecondary);
        sf::FloatRect nb = textNumber.getLocalBounds();
        textNumber.setOrigin(nb.left + nb.width/2.f, nb.top + nb.height/2.f);

        textTitle.setFont(fontBold);
        textTitle.setString(config.title);
        textTitle.setCharacterSize(22);
        textTitle.setFillColor(Theme::Color::TextPrimary);
        sf::FloatRect tb = textTitle.getLocalBounds();
        textTitle.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);

        textBigTitle.setFont(fontBold);
        textBigTitle.setString(config.title);
        textBigTitle.setCharacterSize(80);
        textBigTitle.setFillColor(sf::Color::White);
        sf::FloatRect btb = textBigTitle.getLocalBounds();
        textBigTitle.setOrigin(btb.left + btb.width/2.f, btb.top + btb.height/2.f);
        textBigTitle.setScale(0.f, 0.f);

        // 5. Setup Buttons
        sf::Font& fontReg = res.getFont("assets/fonts/font.ttf");

        btnViewMore = new Button(fontReg, "View More", {100.f, 36.f});
        btnViewMore->applyPreset(ButtonPreset::Ghost);
        btnViewMore->setCornerRadius(18.f);
        btnViewMore->setOutline(1.f, sf::Color::Black);
        btnViewMore->setTextColor(sf::Color::Black);
        btnViewMore->setOpacity(0.0f);

        btnBack = new Button(fontBold, "Back", {80.f, 40.f});
        btnBack->applyPreset(ButtonPreset::Ghost);
        btnBack->setTextColor(sf::Color::White); // Nút Back màu trắng trên nền tối khi expand
        btnBack->setOutline(1.f, sf::Color::White);
        btnBack->setCornerRadius(20.f);
        btnBack->setOpacity(0.0f);

        btnStart = new Button(fontBold, "Start Visualization", {220.f, 60.f});
        btnStart->applyPreset(ButtonPreset::Clean);
        btnStart->setCornerRadius(30.f);
        btnStart->setOpacity(0.0f);

        // 6. Init Callbacks (Gán nullptr cho an toàn)
        onSelect = nullptr;
        onViewMore = nullptr;
        onStart = nullptr;
        onBack = nullptr;

        // 7. Setup Physics
        selectionSpring.stiffness = Theme::Animation::CardStiffness;
        selectionSpring.damping = Theme::Animation::CardDamping;
        selectionSpring.snapTo(0.0f);

        expansionSpring.stiffness = Theme::Animation::CardStiffness;
        expansionSpring.damping = Theme::Animation::CardDamping;
        expansionSpring.snapTo(0.0f);
    }

    MenuCard::~MenuCard()
    {
        delete btnViewMore; delete btnBack; delete btnStart;
    }

    // --- SETTERS IMPLEMENTATION ---
//    void MenuCard::setOnSelect(std::function<void()> callback) { actionSelect = callback; }
//    void MenuCard::setOnViewMore(std::function<void()> callback) { actionViewMore = callback; }
//    void MenuCard::setOnStart(std::function<void()> callback) { actionStart = callback; }
//    void MenuCard::setOnBack(std::function<void()> callback) { actionBack = callback; }

    void MenuCard::snapToTarget()
    {
        // 1. Gán giá trị ngay lập tức
        currentPos = targetPos;
        currentSize = targetSize;

        // 2. Cập nhật Visual (Squircle)
        bgShape.setPosition(currentPos);
        bgShape.setSize(currentSize);

        // 3. [QUAN TRỌNG] Tính lại tâm (Origin)
        // Vì size thay đổi nên tâm cũng dịch chuyển. Nếu không set lại, hình sẽ bị lệch.
        bgShape.setOrigin(currentSize.x / 2.0f, currentSize.y / 2.0f);
    }

//    void MenuCard::setCardPosition(CardPos pos, float radius) { cardPos = pos; cornerRadius = radius; }

    void MenuCard::setCardPosition(CardPos pos)
    {
        this->cardPos = pos;
        // Gọi updateLayout ngay để áp dụng góc bo mới (nếu đang ở trạng thái tĩnh)
        updateLayout(selectionSpring.position, expansionSpring.position);
    }

    void MenuCard::setTarget(const sf::Vector2f& pos, const sf::Vector2f& size)
    {
        targetPos = pos;
        targetSize = size;
    }

    void MenuCard::setSelected(bool sel)
    {
        if (selected != sel)
        {
            selected = sel;
            // [QUAN TRỌNG] Phải set target cho lò xo thì nó mới chạy!
            selectionSpring.target = selected ? 1.0f : 0.0f;
        }
    }
    void MenuCard::setExpanded(bool exp)
    {
        if (expanded != exp)
        {
            expanded = exp;
            // [QUAN TRỌNG] Set target cho lò xo mở rộng
            expansionSpring.target = expanded ? 1.0f : 0.0f;

            // Nếu mở rộng thì mặc định là đang chọn luôn
            if (expanded) setSelected(true);
        }
    }

    sf::FloatRect MenuCard::getGlobalBounds() const { return bgShape.getGlobalBounds(); }

    // --- HANDLE EVENT ---
    void MenuCard::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
//        assert(globalAlpha > 100.f);
        if (globalAlpha < 50.0f)
        {
//            std::cout << "Card ignored due to low alpha: " << globalAlpha << "\n";
            return;
        }

        // 1. Xử lý Start & Back (Khi đã Expand)
        if (expanded && expansionSpring.position > 0.8f) {
            // [ĐÃ SỬA] Gán callback trước, sau đó để nút tự xử lý logic click
            if (onStart) btnStart->onClick = onStart;
            btnStart->handleEvent(event, window);

            if (onBack) btnBack->onClick = onBack;
            btnBack->handleEvent(event, window);
            return;
        }

        // 2. Xử lý View More (Khi chưa Expand)
        if (selected && !expanded && selectionSpring.position > 0.8f) {
            // [ĐÃ SỬA] Gán callback trước
            if (onViewMore) btnViewMore->onClick = onViewMore;
            btnViewMore->handleEvent(event, window);
        }

        // 3. Click Card to Select
        if (!expanded && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::FloatRect bounds = bgShape.getGlobalBounds();

            // [DEBUG 2] In ra tọa độ chuột và tọa độ thẻ
//            std::cout << "Click Pos: " << mPos.x << ", " << mPos.y << "\n";
//            std::cout << "Card Bounds: Left=" << bounds.left << ", Top=" << bounds.top
//                  << ", W=" << bounds.width << ", H=" << bounds.height << "\n";

            if (bounds.contains(mPos))
            {
//                std::cout << "-> HIT CARD!\n"; // Nếu thấy dòng này là logic click đúng
                if (!selected)
                {
//                    std::cout << "-> SET SELECTED TRUE\n";
                    setSelected(true);
                    if (onSelect) onSelect();
                }
            }
            else
            {
//                std::cout << "-> MISS\n"; // Click trượt
            }
        }
    }

    // ---------------------------------------------------------
    // UPDATE LAYOUT (Dùng Easing::lerp và Easing::lerpColor)
    // ---------------------------------------------------------
    void MenuCard::updateLayout(float selectT, float expandT)
    {
        sf::Vector2f center = currentPos;
        float alphaMult = globalAlpha / 255.0f;

        // --- 1. GÓC BO (CORNER MORPHING) ---
        // Biến t dùng chung cho Màu sắc
//        float rawT = std::max(selectT, expandT);
        float t = std::max(selectT, expandT);
//        float s
        float r = Theme::Style::ItemRadius;
        float baseTL = 0, baseTR = 0, baseBR = 0, baseBL = 0;

        switch (cardPos) {
            case CardPos::First:  baseTL = r; baseBL = r; break;
            case CardPos::Last:   baseTR = r; baseBR = r; break;
            case CardPos::Single: baseTL = r; baseTR = r; baseBR = r; baseBL = r; break;
            case CardPos::Middle: break;
        }

        // Dùng Easing::lerp thay vì lambda tự viết
        bgShape.setCornerRadius(
            Utils::Math::Easing::lerp(baseTL, r, expandT),
            Utils::Math::Easing::lerp(baseTR, r, expandT),
            Utils::Math::Easing::lerp(baseBR, r, expandT),
            Utils::Math::Easing::lerp(baseBL, r, expandT)
        );

        // --- 2. MÀU NỀN ---
        // Dùng Easing::lerpColor chuẩn từ file Easing.h
//        sf::C
        sf::Color finalBg = Utils::Math::Easing::lerpColor(Theme::Color::TransparentColor, config.themeColor, t);
        bgShape.setFillColor(applyAlpha(finalBg, alphaMult));

        // --- 3. ICON ANIMATION ---
        float iconOffsetY = -40.0f - (80.0f * expandT);
        float iconScale = std::max(0.0f, selectT) + (0.5f * expandT);

        iconSprite.setScale(iconScale, iconScale);
        iconSprite.setPosition(center.x, center.y + iconOffsetY);
        iconSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255 * alphaMult)));

        // --- 4. TEXT NHỎ ---
//        float smallTextAlpha = std::max(0.0f, selectT) * (1.0f - expandT) * alphaMult;
        float smallTextAlpha = (1.0f - expandT) * alphaMult;
        float slideY = 90.0f * selectT;

        textNumber.setPosition(center.x, center.y + slideY - 15.f);
        textNumber.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(15 * smallTextAlpha * 255)));

        textTitle.setPosition(center.x, center.y + slideY + 15.f);
        textTitle.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(255 * smallTextAlpha)));

        // --- 5. TEXT LỚN (Expand) ---
        float bigTitleAlpha = std::max(0.0f, expandT) * alphaMult;
        textBigTitle.setPosition(center.x, center.y + iconOffsetY + 120.f);
        textBigTitle.setScale(expandT, expandT);
        textBigTitle.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(255 * bigTitleAlpha)));

        // --- 6. BUTTONS ---
        btnViewMore->setPosition({center.x, center.y + slideY + 65.f});
        float viewMoreAlpha = std::max(0.0f, selectT - expandT) * globalAlpha;
        btnViewMore->setOpacity(viewMoreAlpha);

        btnStart->setPosition({center.x, center.y + 180.f});
        float expandedBtnAlpha = std::max(0.0f, expandT) * globalAlpha;
        btnStart->setOpacity(expandedBtnAlpha);

        float halfW = currentSize.x / 2.0f;
        float halfH = currentSize.y / 2.0f;
        btnBack->setPosition({center.x - halfW + 60.f, center.y - halfH + 50.f});
        btnBack->setOpacity(expandedBtnAlpha);
    }


    // ---------------------------------------------------------
    // UPDATE LOOP (Dùng Smoothing::damp)
    // ---------------------------------------------------------
    void MenuCard::update(float dt, sf::RenderWindow& window)
    {
        // 1. Movement Physics
        // Dùng Smoothing::damp từ thư viện của bạn
        float moveSpeed = 0.001f;

        currentPos.x = Utils::Math::Smoothing::damp(currentPos.x, targetPos.x, moveSpeed, dt);
        currentPos.y = Utils::Math::Smoothing::damp(currentPos.y, targetPos.y, moveSpeed, dt);

        currentSize.x = Utils::Math::Smoothing::damp(currentSize.x, targetSize.x, moveSpeed, dt);
        currentSize.y = Utils::Math::Smoothing::damp(currentSize.y, targetSize.y, moveSpeed, dt);

        bgShape.setPosition(currentPos);
        bgShape.setSize(currentSize);
        bgShape.setOrigin(currentSize.x / 2.f, currentSize.y / 2.f);

        // 2. Effects Physics
        selectionSpring.update(dt);
        expansionSpring.update(dt);

        // 3. Update Layout
        updateLayout(selectionSpring.position, expansionSpring.position);

        // 4. Update Buttons
        if (selectionSpring.position > 0.01f && expansionSpring.position < 0.99f) {
            btnViewMore->update(window, dt);
        }
        if (expansionSpring.position > 0.01f) {
            btnStart->update(window, dt);
            btnBack->update(window, dt);
        }

        // [DEBUG] KIỂM TRA GIÁ TRỊ LÒ XO
        // Chỉ in ra log của Card đầu tiên (ID=0) để đỡ bị spam console
//        if (config.id == 0)
//        {
//            // Kiểm tra lỗi NaN (Not a Number - Lỗi nổ vật lý)
//            if (std::isnan(selectionSpring.position) || std::isnan(expansionSpring.position))
//            {
//                std::cout << "[CRITICAL ERROR] Spring bị NaN! Kiểm tra lại dt hoặc stiffness/damping.\n";
//            }
//
//            // Chỉ in khi lò xo đang di chuyển (chưa đến đích)
//            bool isMoving = (std::abs(selectionSpring.target - selectionSpring.position) > 0.001f) ||
//                            (std::abs(expansionSpring.target - expansionSpring.position) > 0.001f);
//
//            if (isMoving)
//            {
//                std::cout << "Card 0 | "
//                          << "Select: " << selectionSpring.position << " -> " << selectionSpring.target
//                          << " | Expand: " << expansionSpring.position << " -> " << expansionSpring.target
//                          << "\n";
//            }
//        }
    }

    // --- DRAW & EVENT (Giữ nguyên logic) ---
    void MenuCard::draw(sf::RenderWindow& window)
    {
        sf::Vector2f viewMorePos = btnViewMore -> getPosition();
//        std::cout <<  viewMorePos.x << " " << viewMorePos.y << "\n";

//        std::cout << currentPos.x << " " << currentPos.y <<

        if (globalAlpha < 1.0f) return;

        window.draw(bgShape);

        if (expansionSpring.position < 0.99f) {
            window.draw(textNumber);
            window.draw(textTitle);
        }

        if (expansionSpring.position > 0.01f) {
            window.draw(textBigTitle);
        }

        window.draw(iconSprite);

        if (selectionSpring.position > 0.05f && expansionSpring.position < 0.95f) {
            btnViewMore->draw(window);
        }
        if (expansionSpring.position > 0.05f) {
            btnStart->draw(window);
            btnBack->draw(window);

        }
    }

//    const sf::Text& MenuCard::getTitleText() const
//    {
//        return textTitle;
//    }
//
//    void MenuCard::setTextVisible(bool visible)
//    {
//        // Chỉnh Alpha của Title
//        sf::Color cTitle = textTitle.getFillColor();
//        cTitle.a = visible ? 255 : 0;
//        textTitle.setFillColor(cTitle);
//
//        // (Tùy chọn) Ẩn luôn số thứ tự (01, 02...) nếu muốn sạch sẽ
//        sf::Color cNum = textNumber.getFillColor();
//        cNum.a = visible ? 255 : 0; // Hoặc để 150 nếu muốn mờ mờ
//        textNumber.setFillColor(cNum);
//    }

//    void MenuCard::setGhostMode(bool enabled)
//    {
//        // 1. Nếu trạng thái không đổi thì không làm gì cả (để tránh spam setFillColor liên tục)
//        if (this->ghostMode == enabled) return;
//
//        this->ghostMode = enabled;
//
//        // 2. [FIX QUAN TRỌNG] Xử lý khi TẮT Ghost Mode
//        if (!enabled)
//        {
//            // Khi vừa tắt Ghost (vừa chui vào bảng), ta muốn nó ĐẶC ngay lập tức.
//            // Ta tính toán màu đích mong muốn (giống logic trong update)
//            sf::Color targetColor = (selected || expanded) ? themeColor : sf::Color::White;
//
//            // Gán TRỰC TIẾP, bỏ qua hàm dampColor để không bị fade từ trong suốt lên
//            bgShape.setFillColor(targetColor);
//        }
//
//        // Nếu BẬT Ghost Mode (enabled == true), ta cứ để hàm update() lo việc fade out từ từ
//        // hoặc nếu muốn tàng hình ngay lập tức thì cũng gán luôn ở đây:
//        else
//        {
//            bgShape.setFillColor(sf::Color::Transparent);
//        }
//    }

    bool MenuCard::isSettled() const
    {
        // Cho phép sai số nhỏ (epsilon) vì so sánh số thực float
        float epsilon = 1.0f; // Sai số 1 pixel là chấp nhận được

        bool posDone = (std::abs(currentPos.x - targetPos.x) < epsilon) &&
                       (std::abs(currentPos.y - targetPos.y) < epsilon);

        bool sizeDone = (std::abs(currentSize.x - targetSize.x) < epsilon) &&
                        (std::abs(currentSize.y - targetSize.y) < epsilon);

        return posDone && sizeDone;
    }

    void MenuCard::setOpacity(float alpha)
    {
        // Giới hạn giá trị từ 0 đến 255 để tránh lỗi hiển thị màu
        globalAlpha = std::max(0.0f, std::min(alpha, 255.0f));
    }

//    void MenuCard::setTextOpacity(float alpha)
//    {
//        // Giới hạn giá trị từ 0 đến 255
////        if(alpha < 0.0f) alpha = 0.0f;
//        if(alpha > 255.0f) alpha = 255.0f;
//
//        textOpacity = alpha;
//    }

//    void MenuCard::setSelectionStyle(bool fillBackground)
//    {
//        this->fillOnSelect = fillBackground;
//        if(!fillOnSelect) btnViewMore->setTextColor(sf::Color::Black);
//    }
}


