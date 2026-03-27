#include "MenuCard.h"
#include "ResourceManager.h"
#include "ViewHandler.h"
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

//        config.colorNumber = config.themeColor;

        // 2. Setup Background
        bgShape.setSize(currentSize);
//        bgShape.setCurvature(Theme::Style::SquircleCurvature);
        bgShape.setOrigin(currentSize.x / 2.f, currentSize.y / 2.f);
        bgShape.setFillColor(Theme::Color::Background);
        bgShape.setRadius(0.f);

        // 3. Setup Icon
        if (config.iconTexture) {
//            config.iconTexture->setSmooth(true);
            iconSprite.setTexture(*config.iconTexture);
            sf::FloatRect iconBounds = iconSprite.getLocalBounds();
            iconSprite.setOrigin(iconBounds.width / 2.f, iconBounds.height / 2.f);
            iconSprite.setScale(0.f, 0.f);
            textureWidth = config.iconTexture -> getSize().x;
        }

        // 4. Setup Texts
        sf::Font& fontBold = res.getFont("assets/fonts/SFProDisplay-Bold.ttf");

        textNumber.setFont(res.getFont("assets/fonts/SFProDisplay-Regular.ttf"));
        textNumber.setString(config.number);
        textNumber.setCharacterSize(50);
        textNumber.setFillColor(Theme::Color::TextSecondary);
        sf::FloatRect nb = textNumber.getLocalBounds();
        textNumber.setOrigin(nb.left + nb.width/2.f, nb.top + nb.height/2.f);

        textTitle.setFont(res.getFont("assets/fonts/SFProText-Semibold.ttf"));
        textTitle.setString(config.title);
        textTitle.setCharacterSize(22);
        const_cast<sf::Texture&>(res.getFont("assets/fonts/SFProText-Semibold.ttf").getTexture(22)).setSmooth(true);
//        textTitle.setScale(0.5f, 0.5f);
        textTitle.setFillColor(Theme::Color::TextPrimary);
        sf::FloatRect tb = textTitle.getLocalBounds();
        textTitle.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);


        // [MỚI] SETUP SHORT DESCRIPTION
        // Dùng font Regular/Medium để tạo độ tương phản với nét Bold của Title
        sf::Font& fontDescSmall = res.getFont("assets/fonts/SFProText-Regular.ttf");
        textShortDesc.setFont(fontDescSmall);
        textShortDesc.setString(config.shortDescription);
        textShortDesc.setCharacterSize(16); // Nhỏ hơn Title (22px) để phân cấp thị giác
        const_cast<sf::Texture&>(fontDescSmall.getTexture(16)).setSmooth(true);

        sf::FloatRect sdb = textShortDesc.getLocalBounds();
        textShortDesc.setOrigin(sdb.left + sdb.width/2.f, sdb.top + sdb.height/2.f);

        textBigTitle.setFont(fontBold);
        textBigTitle.setString(config.title);
        textBigTitle.setCharacterSize(70);
        const_cast<sf::Texture&>(res.getFont("assets/fonts/SFProDisplay-Bold.ttf").getTexture(80)).setSmooth(true); ///******************88

        textBigTitle.setFillColor(sf::Color::White);
        sf::FloatRect btb = textBigTitle.getLocalBounds();
        textBigTitle.setOrigin(btb.left + btb.width/2.f, btb.top + btb.height/2.f);
        textBigTitle.setScale(0.f, 0.f);


        // Thêm vào cuối phần 4. Setup Texts trong constructor
        textDescription.setFont(res.getFont("assets/fonts/SFProText-Regular.ttf")); // Font regular

        textDescription.setString(config.description);
        textDescription.setCharacterSize(20);
        const_cast<sf::Texture&>(res.getFont("assets/fonts/SFProText-Regular.ttf").getTexture(20)).setSmooth(true);
        textDescription.setLineSpacing(1.3f); // Apple-style: giãn dòng thoáng
        textDescription.setFillColor(sf::Color(255, 255, 255, 0)); // Khởi tạo trong suốt

        // Gọi hàm wrap ngay sau khi set font/string
        // Giả sử vùng text chiếm khoảng 40% chiều rộng của Expanded Card (1200 * 0.4 = 480)
        Utils::Graphics::wrapText(textDescription, 480.f);

        // Tính lại Origin cho Description (Top-Left để dễ căn lề trái)
        sf::FloatRect db = textDescription.getLocalBounds();
        textDescription.setOrigin(0.f, 0.f);



        // 5. Setup Buttons
        sf::Font& fontReg = res.getFont("assets/fonts/SFProText-Regular.ttf");

        btnViewMore = new Button(fontReg, "View More", {100.f, 36.f});
        btnViewMore->applyPreset(ButtonPreset::Ghost);
        btnViewMore->setCornerRadius(18.f);
        btnViewMore->setOutline(1.f, Theme::Color::ButtonOutlineColor);
        btnViewMore->setTextColor(sf::Color::Black);
        btnViewMore->setOpacity(0.0f);
        btnViewMore->setPower(2);

        btnBack = new Button(fontBold, "Back", {80.f, 40.f});
        btnBack->applyPreset(ButtonPreset::Ghost);
        btnBack->setTextColor(sf::Color::White); // Nút Back màu trắng trên nền tối khi expand
        btnBack->setOutline(1.f, sf::Color::White);
        btnBack->setCornerRadius(20.f);
        btnBack->setOpacity(0.0f);
        btnBack->setPower(2);

        btnStart = new Button(fontReg, "Start Visualization", {220.f, 60.f});
        btnStart->applyPreset(ButtonPreset::Clean);
        btnStart->setCornerRadius(30.f);
        btnStart->setOpacity(0.0f);
        btnStart->setPower(2);

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
    void MenuCard::handleEvent(const sf::Event& event, sf::RenderWindow& window)
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
//            sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2i mPos = {event.mouseButton.x, event.mouseButton.y};
            sf::FloatRect bounds = bgShape.getGlobalBounds();

            // [DEBUG 2] In ra tọa độ chuột và tọa độ thẻ
//            std::cout << "Click Pos: " << mPos.x << ", " << mPos.y << "\n";
//            std::cout << "Card Bounds: Left=" << bounds.left << ", Top=" << bounds.top
//                  << ", W=" << bounds.width << ", H=" << bounds.height << "\n";

//            if (bounds.contains(mPos))
            if (Utils::ViewHandler::isMouseInFrame(mPos, window, bounds))
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
        float halfW = currentSize.x / 2.0f;
        float halfH = currentSize.y / 2.0f;

        sf::Vector2f center = currentPos;
        float alphaMult = globalAlpha / 255.0f;

        // --- 1. GÓC BO (CORNER MORPHING) ---
        // Biến t dùng chung cho Màu sắc
//        float rawT = std::max(selectT, expandT);
        float t = std::max(selectT, expandT);
//        float s
        float r = config.initialSize.x / Theme::Style::AlgoCardW * Theme::Style::AlgoRadius;
//        std::cout << config.initialSize.x << " " << Theme::Style::AlgoCardW << "\n";
        float baseTL = 0, baseTR = 0, baseBR = 0, baseBL = 0;

        switch (cardPos) {
            case CardPos::First:  baseTL = r; baseBL = r; break;
            case CardPos::Last:   baseTR = r; baseBR = r; break;
            case CardPos::Single: baseTL = r; baseTR = r; baseBR = r; baseBL = r; break;
            case CardPos::Middle: break;
        }

        // Dùng Easing::lerp thay vì lambda tự viết
        bgShape.setRadius(
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

        // --- 3. ICON ANIMATION (Căn phải) ---
        // Tính tọa độ đích bên phải: Cạnh phải của thẻ (center.x + halfW) lùi lại một khoảng Padding
        float iconExpandedX = center.x + halfW - Theme::Style::ExpandedPaddingRight;
        float iconTargetX = Utils::Math::Easing::lerp(center.x, iconExpandedX, expandT);

//        float iconOffsetY = 40 -40.0f - (-20 + 20.0f * expandT);
        float iconOffsetY = 40 -40.0f - (70.f * (1 - expandT));


        // 1. Scale trạng thái chưa Expand (Accordion Mode)
        // Chiều rộng tối đa cho phép = Chiều rộng thẻ - 40px (Padding 20px mỗi bên)
        float maxUnexpandedWidth = std::max(0.0f, config.initialSize.x - 70.0f);
        float targetSelectedScale = maxUnexpandedWidth / textureWidth;

        // Kết hợp hiệu ứng nảy (selectT): Bình thường icon nhỏ gọn (0.8), khi click chọn nảy lên kích thước max (1.0)
        float unexpandedScale = targetSelectedScale * std::max(0.0f, selectT);

        // 2. Scale trạng thái Expand (Hero Mode)
        // Cố định chiều rộng (Ví dụ: 256px). Bạn nên đưa số này vào Theme::Style::IconMaxSize
        float targetExpandedScale = Theme::Style::IconMaxWidth / textureWidth;

//        float safeexpandT = std::max(0.0f, std::min(1.0f, expandT));

        // 3. Nội suy Scale
        float iconScale = Utils::Math::Easing::lerp(unexpandedScale, targetExpandedScale, expandT);


        iconSprite.setScale(iconScale, iconScale);
        iconSprite.setPosition(std::round(iconTargetX), std::round(center.y + iconOffsetY));
        iconSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255 * alphaMult)));


        // --- 4. TEXT NHỎ ---

        // --- 4. TEXT NHỎ (Number, Title & Short Desc) ---
        float slideY = (100.0f / Theme::Style::AlgoCardH) * config.initialSize.y * selectT;
        float baseAlpha = (1.0f - expandT) * alphaMult; // Mờ dần khi Expand

        // 4.1. NUMBER
        float numberAlphaMult = std::max(0.0f, 1.0f - selectT);
        sf::Color numColor = config.colorNumber;
        numColor.a = (sf::Uint8)(std::max(0.f, std::min(255.f, 255 * baseAlpha * numberAlphaMult)));

        textNumber.setPosition(std::round(center.x), std::round(center.y + slideY - 15.f));
        textNumber.setFillColor(numColor);

        // 4.2. TITLE (Dùng Data-Driven Color)
        sf::Color currentTitleColor = Utils::Math::Easing::lerpColor(
            config.colorTitleUnselected,
            config.colorTitleSelected,
            selectT
        );
        currentTitleColor.a = (sf::Uint8)(std::max(0.f, std::min(255.f, 255 * baseAlpha)));

        textTitle.setPosition(std::round(center.x), std::round(center.y + slideY + 15.f));
        textTitle.setFillColor(currentTitleColor);

        // 4.3. SHORT DESCRIPTION
        // Lấy cùng tông màu với Title hiện tại, nhưng giảm Alpha xuống khoảng 60-70% để nhường sự chú ý
        sf::Color shortDescColor = currentTitleColor;
        shortDescColor.a = (sf::Uint8)(std::max(0.f, std::min(255.f, 160 * baseAlpha)));

        // Đặt cách Title khoảng 28px về phía dưới
        textShortDesc.setPosition(std::round(center.x), std::round(center.y + slideY + 43.f));
        textShortDesc.setFillColor(shortDescColor);

//        float slideY = (150.0f / Theme::Style::AlgoCardH) * config.initialSize.y * selectT;
//        float baseAlpha = (1.0f - expandT) * alphaMult; // Tổng thể mờ đi khi Expand
//
//        // Xử lý Number: Mờ dần khi được Select (1.0 -> 0.0)
//        float numberAlphaMult = std::max(0.0f, 1.0f - selectT);
//        sf::Color numColor = config.themeColor;
//        numColor.a = (sf::Uint8)(std::max(0.f, std::min(255.f, 255 * baseAlpha * numberAlphaMult)));
//
        textNumber.setPosition(std::round(center.x), std::round(center.y + slideY - 15.f - 75.f));
//        textNumber.setFillColor(numColor);
//
//        // Xử lý Title: Xám đậm (#1D1D1F) -> Trắng khi được Select
//        sf::Color titleUnselected(29, 29, 31);
//        sf::Color titleSelected(255, 255, 255);
//
//        // Lerp màu từ xám sang trắng dựa trên selectT
//        sf::Color currentTitleColor = Utils::Math::Easing::lerpColor(titleUnselected, titleSelected, selectT);
//        currentTitleColor.a = (sf::Uint8)(std::max(0.f, std::min(255.f, 255 * baseAlpha))); // Áp dụng alpha tổng
//
        textTitle.setPosition(std::round(center.x), std::round(center.y + slideY + 15.f));
//        textTitle.setFillColor(currentTitleColor);


        // --- 5. TEXT LỚN & DESCRIPTION ---
        float textLeftX = center.x - halfW + Theme::Style::ExpandedPaddingLeft;

        float titleX = Utils::Math::Easing::lerp(center.x, textLeftX, expandT);
        float titleY = center.y + Theme::Style::TitleOffsetY;

        sf::FloatRect btb = textBigTitle.getLocalBounds();
        if(btb.width > 0)
        {
            if(expandT > 0.01f)
            {
                textBigTitle.setOrigin(0.f, btb.top + btb.height / 2.f);
            }
            else
            {
                textBigTitle.setOrigin(btb.left + btb.width / 2.f, btb.top + btb.height / 2.f);
            }
        }

        textBigTitle.setPosition(std::round(titleX), std::round(titleY));
        textBigTitle.setScale(0.6f + 0.4f * expandT, 0.6f + 0.4f * expandT);

        sf::Color colorTransparent(255, 255, 255, 0);
        sf::Color targetTitleColor(255, 255, 255, (sf::Uint8)(std::max(0.f, std::min(255.f, 255 * alphaMult))));
        textBigTitle.setFillColor(Utils::Math::Easing::lerpColor(colorTransparent, targetTitleColor, expandT));

        // DESCRIPTION
        sf::Color targetDescColor(255, 255, 255, (sf::Uint8)(std::max(0.f, std::min(255.f, 180 * alphaMult))));
        float safeExpandT = std::max(0.0f, std::min(1.0f, expandT));
        float descT = std::max(0.0f, std::min(1.0f, (safeExpandT - 0.5f) * 2.0f));

        textDescription.setPosition(std::round(titleX), std::round(titleY + Theme::Style::DescSpacingY));
        textDescription.setFillColor(Utils::Math::Easing::lerpColor(colorTransparent, targetDescColor, descT));

        // --- 6. BUTTONS ---
        btnViewMore->setPosition(sf::Vector2f(std::round(center.x), std::round(center.y + slideY + 90.f)));
        float viewMoreAlpha = std::max(0.0f, selectT - expandT) * globalAlpha;
        btnViewMore->setOpacity(viewMoreAlpha);

        // NÚT START: Căn theo lề trái (titleX) + offset để vào đúng trục
        // btnStart->getScale() không đổi nên ta cộng half-width của nút (vd: 220/2 = 110)
        btnStart->setPosition(sf::Vector2f(std::round(titleX + 110.f), std::round(titleY + Theme::Style::ButtonSpacingY)));
        float expandedBtnAlpha = std::max(0.0f, expandT) * globalAlpha;
        btnStart->setOpacity(expandedBtnAlpha);

        // NÚT BACK: Căn thẳng tắp với lề trái của textBigTitle
        // Nút Back size {80, 40}, Origin ở tâm -> half-width = 40.f
        float backTargetX = textLeftX + 40.f;
        float backTargetY = center.y - halfH + Theme::Style::ExpandedPaddingTop;
        btnBack->setPosition(sf::Vector2f(std::round(backTargetX), std::round(backTargetY)));
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
            window.draw(textShortDesc);
        }

        if (expansionSpring.position > 0.01f) {
//             std::cout << "BigTitle Pos: " << textBigTitle.getPosition().x << ", " << textBigTitle.getPosition().y
//                       << " | Alpha: " << (int)textBigTitle.getFillColor().a << "\n";
//             std::cout << "Desc Alpha: " << (int)textDescription.getFillColor().a << "\n";

            window.draw(textBigTitle);
            if(expansionSpring.position > 0.5f) // Chỉ vẽ khi bắt đầu hiện rõ
            {
                window.draw(textDescription);
            }
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


