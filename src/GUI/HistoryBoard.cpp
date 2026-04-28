// GUI/HistoryBoard.cpp
#include "ViewHandler.h"
#include "GUI/HistoryBoard.h"
#include "Snapshot.h" // Để lấy macroStepID và logMessage
#include "Easing.h"
#include "ResourceManager.h"
#include <iostream>

namespace GUI
{
    HistoryBoard::HistoryBoard()
    {
        // Khởi tạo nền Squircle
        m_background.setSize({m_width, m_height});
        m_background.setOrigin(m_width, m_height);
        m_background.setRadius(30.f);
        m_background.setFillColor(sf::Color(25, 25, 25, 230)); // Đen mờ Glassmorphism
        m_background.setOutlineThickness(1.f);
        m_background.setOutlineColor(sf::Color(255, 255, 255, 40)); // Viền trắng cực mảnh

        m_scaleSpring.stiffness = 350.f;
        m_scaleSpring.damping = 25.f;
        m_scaleSpring.snapTo(0.f);
    }

    bool HistoryBoard::containsMouse(const sf::RenderWindow& window) const
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        // mapPixelToCoords sẽ tự xử lý sai lệch do Letterbox cho em
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePos);

        // getGlobalBounds() kết hợp với getTransform() sẽ cho ra vùng chuẩn của Board
        // ngay cả khi Origin nằm ở góc phải dưới (vùng tọa độ âm)
        sf::FloatRect bounds = getTransform().transformRect(m_background.getGlobalBounds());

        return bounds.contains(mouseWorld);
    }

int HistoryBoard::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
{
    // 1. Chặn tương tác khi Board đang đóng hoặc đang bay nửa chừng
    if(!m_isOpen || m_animationProgress < 0.5f) return -1;

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect boardBounds = getTransform().transformRect(m_background.getGlobalBounds());

    // --- SETUP VIEW CHO TỌA ĐỘ ÂM ---
    // Frame View (Dùng cho Scrollbar)
    sf::View frameView;
    frameView.setSize(m_width, m_height);
    frameView.setCenter(-m_width / 2.f, -m_height / 2.f); // Soi vào vùng âm
    sf::Vector2f frameMouse = Utils::ViewHandler::mapPixelToWorld(mousePos, window, boardBounds, frameView);

    // Content View (Dùng cho các dòng chữ)
    sf::View contentView = frameView;
    // Soi vào vùng âm và cộng thêm độ cuộn (scroll)
    contentView.setCenter(-m_width / 2.f, -m_height / 2.f + m_scrollSpring.position);
    sf::Vector2f contentMouse = Utils::ViewHandler::mapPixelToWorld(mousePos, window, boardBounds, contentView);

    // 2. XỬ LÝ SCROLLBAR (Kéo/Thả)
    if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        m_scrollbar.setDragging(false);
    }

    if(m_scrollbar.isDragging())
    {
        m_scrollbar.show();
        float thumbH = m_scrollbar.getThumbHeight();
        float trackSpace = m_height - thumbH;

        if(trackSpace > 0.f)
        {
            // Chuyển frameMouse.y từ âm sang dương tương đối (0 -> m_height)
            float relativeY = frameMouse.y + m_height;

            float newThumbY = relativeY - m_scrollbar.getDragOffset();
            float scrollPercent = std::clamp(newThumbY / trackSpace, 0.f, 1.f);
            float targetScroll = scrollPercent * std::max(0.f, m_totalCalculatedHeight - m_height);

            m_scrollSpring.target = targetScroll;
            m_scrollSpring.snapTo(targetScroll);
        }
        return -1;
    }

    // Nếu chuột nằm ngoài Board thì thôi
    if(!Utils::ViewHandler::isMouseInFrame(mousePos, window, boardBounds)) return -1;

    // 3. CUỘN CHUỘT (Mouse Wheel)
    if(event.type == sf::Event::MouseWheelScrolled)
    {
        m_scrollbar.show();
        float delta = event.mouseWheelScroll.delta * 60.f;
        m_scrollSpring.target -= delta;
        return -1;
    }

    // 4. XỬ LÝ CLICK CHUỘT
    if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        // A. Click trúng Thumb của Scrollbar
        if(m_scrollbar.isMouseOverThumb(frameMouse))
        {
            m_scrollbar.setDragging(true);
            // Tính offset dựa trên tọa độ dương tương đối
            float relativeY = frameMouse.y + m_height;
            m_scrollbar.setDragOffset(relativeY - m_scrollbar.getThumbY());
            return -1;
        }

        // B. Click vào các Row trong danh sách
        float currentY = -m_height + 25.f; // Bắt đầu từ đỉnh bảng (Vùng âm)

        // Lề trái của bảng là -m_width, vùng mũi tên rộng 50px
        const float arrowZoneRightBoundary = -m_width + 50.f;

        for(auto& op : m_uiRecords)
        {
            float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);
            float headerY = currentY;

            // KIỂM TRA CLICK VÀO HEADER (Rect bắt đầu từ -m_width)
            sf::FloatRect headerRect(-m_width, headerY, m_width, m_rowHeight);

            if(headerRect.contains(contentMouse))
            {
                // Nếu click vào bên trái của Boundary (Vùng mũi tên)
                if(contentMouse.x < arrowZoneRightBoundary)
                {
                    op.isExpanded = !op.isExpanded;
                    return -1;
                }
                else
                {
                    return op.finalSnapshotIndex;
                }
            }

            currentY += m_rowHeight;

            // KIỂM TRA CLICK VÀO SUB-STEPS
            for(size_t i = 0; i < op.subSteps.size(); ++i)
            {
                float targetOffset = (i + 1) * m_rowHeight;
                float animatedSubY = headerY + (targetOffset * visualT);

                // Rect bắt đầu từ lề trái âm (-m_width)
                sf::FloatRect subRect(-m_width, animatedSubY, m_width, m_rowHeight);

                if(visualT > 0.5f && subRect.contains(contentMouse))
                {
                    return op.subSteps[i].snapshotIndex;
                }
            }

            // Tính lại currentY dựa trên độ giãn thực tế
            currentY = headerY + m_rowHeight + (op.subSteps.size() * m_rowHeight * visualT);
        }
    }

    return -1;
}

    void HistoryBoard::updateScrollPhysics(float dt)
    {
        // Chặn biên và tạo hiệu ứng nảy (Đã sửa để không phụ thuộc FPS)
        if(m_scrollSpring.target < 0.f)
        {
            m_scrollSpring.target = Utils::Math::Easing::lerp(m_scrollSpring.target, 0.f, 15.f * dt);
        }
        else if(m_scrollSpring.target > m_maxScrollY)
        {
            m_scrollSpring.target = Utils::Math::Easing::lerp(m_scrollSpring.target, m_maxScrollY, 15.f * dt);
        }

        // Cập nhật vị trí thực tế dựa trên vận tốc lò xo
        m_scrollSpring.update(dt);
    }
//
//    void HistoryBoard::updateOpenAnimation(float dt)
//    {
//        // 1. Cập nhật progress đóng/mở tổng thể
//        if(m_isOpen)
//        {
//            if(m_animationProgress < 1.f) m_animationProgress += dt * m_animationSpeed;
//        }
//        else
//        {
//            if(m_animationProgress > 0.f) m_animationProgress -= dt * m_animationSpeed;
//        }
//        m_animationProgress = std::clamp(m_animationProgress, 0.f, 1.f);
//
//        // 2. Tính toán vị trí Y dựa trên Easing
//        float offsetDist = 200.f; // Khoảng cách bay
//        float t = Utils::Math::Easing::easeOutBack(m_animationProgress);
//        float currentBoardY = Utils::Math::Easing::lerp(m_anchorPos.y + offsetDist, m_anchorPos.y, t);
//
//        // 3. Cập nhật vị trí thực tế của Board
//        sf::Transformable::setPosition(m_anchorPos.x, currentBoardY);
//    }

    void HistoryBoard::updateHoverAndCursor(sf::RenderWindow& window)
    {
        bool isOverClickable = false;
        m_hoveredRowIdx = -1;

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect boardBounds = getTransform().transformRect(m_background.getGlobalBounds());

        if(Utils::ViewHandler::isMouseInFrame(mousePos, window, boardBounds))
        {
            sf::View contentView;
            contentView.setSize(m_width, m_height);
            contentView.setCenter(-m_width / 2.f, -m_height / 2.f + m_scrollSpring.position);
            sf::Vector2f mouseInContent = Utils::ViewHandler::mapPixelToWorld(mousePos, window, boardBounds, contentView);

            sf::View frameView;
            frameView.setSize(m_width, m_height);
            frameView.setCenter(-m_width / 2.f, -m_height / 2.f);
            sf::Vector2f mouseInFrame = Utils::ViewHandler::mapPixelToWorld(mousePos, window, boardBounds, frameView);

            if(m_scrollbar.isMouseOverThumb(mouseInFrame) || m_scrollbar.isDragging())
            {
                isOverClickable = true;
            }
            else
            {
                // SỬA TẠI ĐÂY: checkY âm
                float checkY = -m_height + 25.f;
                int rowCounter = 0;

                for(const auto& op : m_uiRecords)
                {
                    float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);

                    if(sf::FloatRect(-m_width, checkY, m_width, m_rowHeight).contains(mouseInContent))
                    {
                        m_hoveredRowIdx = rowCounter;
                        isOverClickable = true;
                    }
                    rowCounter++;

                    float subHeightTotal = op.subSteps.size() * m_rowHeight * visualT;
                    if(visualT > 0.1f && !isOverClickable)
                    {
                        for(size_t i = 0; i < op.subSteps.size(); ++i)
                        {
                            float subY = checkY + m_rowHeight + (i * m_rowHeight * visualT);
                            if(sf::FloatRect(-m_width, subY, m_width, m_rowHeight).contains(mouseInContent))
                            {
                                m_hoveredRowIdx = rowCounter;
                                isOverClickable = true;
                                break;
                            }
                            rowCounter++;
                        }
                    }
                    else rowCounter += (int)op.subSteps.size();

                    checkY += m_rowHeight + subHeightTotal;
                    if(isOverClickable) break;
                }
            }
        }

        // Đổi Cursor
        if(isOverClickable && !m_wasLastFrameHand)
        {
            window.setMouseCursor(ResourceManager::getInstance().getCursor(CursorType::Hand));
            m_wasLastFrameHand = true;
        }
        else if(!isOverClickable && m_wasLastFrameHand)
        {
            window.setMouseCursor(ResourceManager::getInstance().getCursor(CursorType::Arrow));
            m_wasLastFrameHand = false;
        }
    }


    void HistoryBoard::update(float dt, sf::RenderWindow& window, int currentManagerIdx)
    {
        if(currentManagerIdx != m_activeSnapshotIdx)
        {
            m_activeSnapshotIdx = currentManagerIdx;

            // --- LOGIC TỰ ĐỘNG ĐÓNG CỤM MACRO ---
            if(!m_uiRecords.empty())
            {
                for(size_t i = 0; i < m_uiRecords.size(); ++i)
                {
//                    // 1. Nếu là cụm cuối cùng -> Luôn giữ mở
//                    if(i == m_uiRecords.size() - 1)
//                    {
//                        m_uiRecords[i].isExpanded = true;
//                        continue;
//                    }

                    // 2. Kiểm tra xem cụm nà++y có chứa snapshot đang active không
                    bool containsActive = false;
                    for(const auto& sub : m_uiRecords[i].subSteps)
                    {
                        if(sub.snapshotIndex == m_activeSnapshotIdx)
                        {
                            containsActive = true;
                            break;
                        }
                    }

                    // 3. Nếu không chứa active và không phải cuối -> Đóng lại
                    if(!containsActive)
                    {
                        m_uiRecords[i].isExpanded = false;
                    }
                    else
                    {
                        m_uiRecords[i].isExpanded = true;
                    }
                }
            }

            autoScrollToActive();
        }

        // 1. Animation đóng/mở Board (Pop-over)
        updateOpenAnimation(dt); // Giả sử ông tách ra hàm này cho gọn

        if(m_animationProgress <= 0.01f) return;

        // 2. CẬP NHẬT TRẠNG THÁI NỘI BỘ
        // Cập nhật tiến trình trượt của từng hàng
        for(auto& op : m_uiRecords)
        {
            float target = op.isExpanded ? 1.f : 0.f;
            op.expansionProgress = Utils::Math::Easing::lerp(op.expansionProgress, target, 8.f * dt);
            if(std::abs(op.expansionProgress - target) < 0.001f) op.expansionProgress = target;
        }

        // GỌI HÀM CỦA ÔNG Ở ĐÂY: Tính toán lại "thước đo"
        updateTotalHeight();

        // 3. VẬT LÝ CUỘN (Rubber Band)
        updateScrollPhysics(dt);

        // 4. LOGIC HOVER & CURSOR
        updateHoverAndCursor(window);

        // 5. CẬP NHẬT SCROLLBAR
        // Lấy tọa độ chuột local để scrollbar check hover trên chính nó
        sf::Vector2f mouseInFrame = getMouseInFrame(window);
        m_scrollbar.update(dt, m_scrollSpring.position, m_totalCalculatedHeight, m_height, m_width, mouseInFrame);
    }

    sf::Vector2f HistoryBoard::getMouseInFrame(sf::RenderWindow& window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect boardBounds = getTransform().transformRect(m_background.getGlobalBounds());

        sf::View frameView;
        frameView.setSize(m_width, m_height);

        // SỬA TẠI ĐÂY: View phải soi vào vùng âm (-m_width đến 0)
        // Tâm của vùng từ -m_width đến 0 là -m_width / 2
        frameView.setCenter(-m_width / 2.f, -m_height / 2.f);

        return Utils::ViewHandler::mapPixelToWorld(mousePos, window, boardBounds, frameView);
    }

    void HistoryBoard::autoScrollToActive()
    {
        if(m_scrollbar.isDragging()) return;

        float currentY = -m_height + 25.f; // Bắt đầu từ vùng âm

        for(auto& op : m_uiRecords)
        {
            float headerY = currentY;
            bool containsActive = false;
            int activeSubIdxInOp = -1;

            for(size_t i = 0; i < op.subSteps.size(); ++i)
            {
                if(op.subSteps[i].snapshotIndex == m_activeSnapshotIdx)
                {
                    containsActive = true;
                    activeSubIdxInOp = (int)i;
                    break;
                }
            }

            if(containsActive)
            {
                if(!op.isExpanded) op.isExpanded = true;
                float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);
                float targetRowY = headerY + m_rowHeight + (activeSubIdxInOp * m_rowHeight * visualT);

                // Căn giữa logic: Chuyển targetRowY âm sang hệ dương tương đối để tính Scroll Target
                float targetRowY_pos = targetRowY + m_height;
                float targetScroll = targetRowY_pos - (m_height / 2.f) + (m_rowHeight / 2.f);

                m_scrollSpring.target = std::clamp(targetScroll, 0.f, m_maxScrollY);
                m_scrollbar.show();
                return;
            }

            float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);
            currentY += m_rowHeight + (op.subSteps.size() * m_rowHeight * visualT);
        }
    }

    void HistoryBoard::setPosition(float x, float y)
    {
        m_anchorPos = {x, y};
        // Gọi hàm của lớp cha để thực sự thay đổi vị trí ngay lập tức
        sf::Transformable::setPosition(x, y);
    }

    void HistoryBoard::setPosition(const sf::Vector2f& pos)
    {
        setPosition(pos.x, pos.y);
    }

    // Sửa drawRow: Nhận trực tiếp font để không phụ thuộc vào ResourceManager lúc draw
    void HistoryBoard::drawRow(sf::RenderTarget& target, sf::RenderStates states,
                               const std::string& textStr, const sf::String& iconUnicode,
                               float yPos, bool isHeader, bool isActive, sf::Uint8 alpha,
                               float rotation, const sf::Font& iconF, const sf::Font& textF) const
    {
        sf::Text icon, label;

        icon.setFont(iconF);
        label.setFont(textF);

        icon.setCharacterSize(isHeader ? 20 : 16);
        label.setCharacterSize(isHeader ? 14 : 12);

        icon.setString(iconUnicode);
        label.setString(sf::String::fromUtf8(textStr.begin(), textStr.end()));

        sf::Color baseColor = (isHeader || isActive) ? sf::Color::White : sf::Color(180, 180, 180);
        baseColor.a = alpha;
        icon.setFillColor(baseColor);
        label.setFillColor(baseColor);

        // Origin & Rotation
        sf::FloatRect iconBounds = icon.getLocalBounds();
        icon.setOrigin(iconBounds.left + iconBounds.width / 2.f, iconBounds.top + iconBounds.height / 2.f);
        icon.setRotation(rotation);

        float startX = isHeader ? 20.f : 20.f + m_indentSize;
        float iconY = yPos + (m_rowHeight / 2.f);
        icon.setPosition(startX, iconY);

        float labelX = startX + (iconUnicode.isEmpty() ? 0.f : 25.f);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(0.f, labelBounds.top + labelBounds.height / 2.f);
        label.setPosition(labelX, iconY);

        target.draw(icon, states);
        target.draw(label, states);
    }


    void HistoryBoard::updateTotalHeight()
    {
        // Bắt đầu với khoảng đệm phía trên (Padding top)
        float total = 25.f;

        for(const auto& op : m_uiRecords)
        {
            // 1. Cộng chiều cao của Operation Header (luôn hiển thị)
            total += m_rowHeight;

            // 2. Cộng chiều cao của các Sub-steps dựa trên tiến trình giãn nở
            // Công thức: (Số lượng con * Chiều cao mỗi dòng) * Progress (0.0 -> 1.0)
            // Dùng Quart Easing để chiều cao co giãn mượt theo hiệu ứng trượt
            float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);
            float subStepsHeight = (op.subSteps.size() * m_rowHeight) * visualT;
            total += subStepsHeight;
        }

        // 3. Cập nhật biến thành viên để Scrollbar và Draw sử dụng
        m_totalCalculatedHeight = total;

        // 4. Tính toán giới hạn cuộn tối đa (Max Scroll Offset)
        // Nó bằng: Tổng chiều cao - Chiều cao khung nhìn + Padding bottom
        m_maxScrollY = std::max(0.f, m_totalCalculatedHeight - m_height + 25.f);
    }



    void HistoryBoard::syncWithManager(const Core::TimelineManager& manager)
    {
        // 1. Xóa sạch UI cũ
        m_uiRecords.clear();

        for(int i=0;i<manager.getCount();++i)
        {
            auto snap = manager.getSnapshot(i);
            if(!snap)
            {
                continue;
            }

            // SỬA TẠI ĐÂY: snap->operationName chuyển thành snap->notchData.title
            // Kiểm tra xem Snapshot này có thuộc về Operation hiện tại không?
            if(m_uiRecords.empty() || m_uiRecords.back().macroID != snap->macroStepID)
            {
                Core::HistoryOperation newOp;
                newOp.macroID = snap->macroStepID;
                newOp.name = snap->notchData.title; // Lấy từ notchData
                newOp.iconCode = snap->notchData.iconCode; // Ông có thể lấy luôn iconCode từ Snapshot
                newOp.isExpanded = false;
                newOp.expansionProgress = 0.f;

                m_uiRecords.push_back(newOp);
            }

            // 2. THÊM SUBSTEP
            Core::HistorySubStep sub;
            // SỬA TẠI ĐÂY: snap->logMessage chuyển thành snap->notchData.subtitle
            sub.description = snap->notchData.subtitle;
            sub.snapshotIndex = i;

            m_uiRecords.back().subSteps.push_back(sub);

            // Cập nhật Snapshot cuối cùng của Operation này
            m_uiRecords.back().finalSnapshotIndex = i;
        }

        // 3. Cập nhật lại layout
        this->updateTotalHeight();
    }

    void HistoryBoard::drawHoverEffect(sf::RenderTarget& target, float yPos) const
    {
        // Vẽ một dải sáng mờ cực nhẹ khi di chuột qua
        GUI::Squircle hoverShape;
        hoverShape.setSize({m_width - 20.f, m_rowHeight - 4.f});
        hoverShape.setRadius(10.f);
        hoverShape.setFillColor(sf::Color(255, 255, 255, 15)); // Trắng cực mờ
        hoverShape.setPosition(10.f, yPos + 2.f);

        target.draw(hoverShape);
    }

//    void HistoryBoard::toggle()
//    {
//        m_isOpen = !m_isOpen;
//
//        // Khi mở bảng, chúng ta thường kích hoạt một số hiệu ứng phụ
//        if(m_isOpen)
//        {
//            // Hiện thanh cuộn ngay lập tức để người dùng định vị được danh sách
//            m_scrollbar.show();
//
//            // Tự động cuộn đến bước đang active để người dùng không phải tìm
//            autoScrollToActive();
//        }
//    }

    void HistoryBoard::toggle()
    {
        m_isOpen = !m_isOpen;

        if (m_isOpen)
        {
            m_scaleSpring.target = 1.0f; // Bung ra
            m_scrollbar.show();
        }
        else
        {
            m_scaleSpring.target = 0.0f; // Thu lại
        }

        std::cout << "Board state: " << (m_isOpen ? "OPEN" : "CLOSED") << std::endl;
    }

    void HistoryBoard::updateOpenAnimation(float dt)
    {
        m_scaleSpring.update(dt);

        m_animationProgress = m_scaleSpring.position;

//        setPosition(m_boardSpring.position);

        float s = m_scaleSpring.position;

        // Hiệu ứng "Bung góc": Chiều cao (Y) nở nhanh hơn chiều ngang (X) một chút
        // tạo cảm giác panel đang vươn lên trên trước khi mở rộng sang trái
        setScale(0.4f + 0.6f * s, s);

        // Độ mờ màng
        sf::Uint8 alpha = static_cast<sf::Uint8>(std::clamp(s * 230.f, 0.f, 230.f));
        m_background.setFillColor(sf::Color(25, 25, 25, alpha));
    }

    void HistoryBoard::drawActiveEffect(sf::RenderTarget& target, float yPos, bool isActive) const
    {
        GUI::Squircle activeShape;
        activeShape.setSize({m_width - 20.f, m_rowHeight - 4.f});
        activeShape.setRadius(10.f);

        if(isActive)
        {
            // Màu xanh Apple đặc trưng cho dòng đang "Active"
            activeShape.setFillColor(sf::Color(0, 122, 255, 40));
            activeShape.setOutlineColor(sf::Color(0, 122, 255, 100));
            activeShape.setOutlineThickness(1.f);
        }
        else
        {
            // Nếu chỉ là hover trên dòng sub-step
            activeShape.setFillColor(sf::Color(255, 255, 255, 10));
        }

        activeShape.setPosition(10.f, yPos + 2.f);
        target.draw(activeShape);
    }

    void HistoryBoard::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if(m_animationProgress < 0.01f || !m_iconFont || !m_textFont) return;

        // 1. Vẽ nền Squircle (Sử dụng transform của Board)
        sf::RenderStates boardStates = states;
        boardStates.transform *= getTransform();
        target.draw(m_background, boardStates);

        // 2. TÍNH TOÁN VIEWPORT CHUẨN (Fix lệch chữ)
        sf::View originalView = target.getView();
        sf::Vector2u winSize = target.getSize();

        // Lấy vùng giới hạn của Board trong hệ 1920x1080
        sf::FloatRect logicBounds = getTransform().transformRect(m_background.getGlobalBounds());

        // Chuyển đổi: Tọa độ 1920x1080 -> Pixel thực trên màn hình (đã tính Letterbox)
        sf::Vector2i topLeftPx = target.mapCoordsToPixel({logicBounds.left, logicBounds.top});
        sf::Vector2i bottomRightPx = target.mapCoordsToPixel({logicBounds.left + logicBounds.width, logicBounds.top + logicBounds.height});

        // Tính tỉ lệ Viewport (0.0 -> 1.0) dựa trên pixel thực
        float vX = (float)topLeftPx.x / winSize.x;
        float vY = (float)topLeftPx.y / winSize.y;
        float vW = (float)(bottomRightPx.x - topLeftPx.x) / winSize.x;
        float vH = (float)(bottomRightPx.y - topLeftPx.y) / winSize.y;

        sf::View contentView;
        contentView.setSize(m_width, m_height);
        contentView.setCenter(m_width / 2.f, (m_height / 2.f) + m_scrollSpring.position);
        contentView.setViewport(sf::FloatRect(vX, vY, vW, vH));

        target.setView(contentView);


        // 3. VẼ NỘI DUNG (Accordion Slide Logic)
        // boardT: Tiến trình hiện nội dung (chỉ bắt đầu hiện khi Board nở được 30%)
        float boardT = std::clamp((m_animationProgress - 0.3f) / 0.7f, 0.f, 1.f);

        // ĐÂY LÀ BIẾN ÔNG ĐANG THIẾU:
        sf::Uint8 boardContentAlpha = static_cast<sf::Uint8>(boardT * 255.f);

        // Hiệu ứng trượt nhẹ từ dưới lên
        float slideOffset = (1.f - boardT) * 20.f;

        float currentY = 25.f + slideOffset;
        int rowCounter = 0;
        float viewTop = m_scrollSpring.position;
        float viewBottom = viewTop + m_height;

        for(const auto& op : m_uiRecords)
        {
            // Áp dụng Easing cho tiến trình trượt của Accordion
            float visualT = Utils::Math::Easing::easeInOutQuart(op.expansionProgress);
            float headerY = currentY;

            // --- VẼ OPERATION HEADER ---
            if(currentY + m_rowHeight > viewTop && currentY < viewBottom)
            {
                // Nếu Board chưa bung đủ (boardT > 0), mới bắt đầu vẽ
                if(boardContentAlpha > 0)
                {
                    if(rowCounter == m_hoveredRowIdx)
                    {
                        // Hover effect cũng cần mờ theo Board
                        drawHoverEffect(target, currentY);
                    }

                    // 1. Icon xoay theo visualT (0 -> 90 độ)
                    float iconRotation = visualT * 90.f;

                    // 2. Vẽ Header: dùng boardContentAlpha để hiện dần ra cùng với Board
                    drawRow(target, sf::RenderStates::Default, op.name, L"\uE13A",
                            currentY, true, false, boardContentAlpha, iconRotation,
                            *m_iconFont, *m_textFont);
                }
            }

            // Tính Alpha cho các dòng con (Sub-steps)
            // Nó là sự kết hợp của: Độ mở Folder (op.expansionProgress) x Độ bung của Board (boardT)
            float subAlphaFactor = std::clamp((op.expansionProgress - 0.3f) / 0.7f, 0.f, 1.f);
            sf::Uint8 rowAlpha = static_cast<sf::Uint8>(subAlphaFactor * boardContentAlpha);

            currentY += m_rowHeight;
            rowCounter++;

            // --- VẼ SUB-STEPS (Hiệu ứng trượt) ---
            // Chúng ta duyệt qua subSteps ngay cả khi visualT = 0 để đảm bảo rowCounter luôn chính xác
            for(size_t i = 0; i < op.subSteps.size(); ++i)
            {
                const auto& sub = op.subSteps[i];

                // Vị trí trượt: Bắt đầu từ headerY và giãn dần ra đến vị trí đích (i+1)
                float targetOffset = (i + 1) * m_rowHeight;
                float animatedY = headerY + (targetOffset * visualT);

                // Chỉ thực sự vẽ nếu visualT đủ lớn để nhìn thấy và nằm trong Viewport
                if(visualT > 0.01f && animatedY + m_rowHeight > viewTop && animatedY < viewBottom)
                {
                    bool isActive = (sub.snapshotIndex == m_activeSnapshotIdx);

                    if(isActive || rowCounter == m_hoveredRowIdx)
                    {
                        drawActiveEffect(target, animatedY, isActive);
                    }

                    // THRESHOLD: Chỉ vẽ khi progress > 0.05 để tránh đè chữ lúc bắt đầu trượt
                    if(op.expansionProgress > 0.3f)
                    {
                        bool isActive = (op.subSteps[i].snapshotIndex == m_activeSnapshotIdx);

                        // Truyền rowAlpha vào drawRow để làm mờ
                        drawRow(target, sf::RenderStates::Default, op.subSteps[i].description,
                                L"\uECE0", animatedY, false, isActive, rowAlpha, 0,
                                *m_iconFont, *m_textFont);
                    }
//                    drawRow(target, sf::RenderStates::Default, sub.description, L"\uF017", animatedY, false, isActive);
                }

                rowCounter++;
            }

            // Cập nhật currentY cho Operation tiếp theo:
            // Nó phải nằm dưới Header và toàn bộ phần Sub-steps đã giãn nở
            currentY = headerY + m_rowHeight + (op.subSteps.size() * m_rowHeight * visualT);
        }


        // 4. VẼ EMPTY STATE (Nếu không có dữ liệu)
        if(m_uiRecords.empty())
        {
            auto& res = ResourceManager::getInstance();
            sf::Text emptyIcon, emptyMsg, subMsg;

            // Thiết lập Font và Size
            emptyIcon.setFont(*m_iconFont);
            emptyMsg.setFont(*m_textFont);
            subMsg.setFont(*m_textFont);

            emptyIcon.setString(L"\uF15B"); // Icon tờ giấy/lịch sử
            emptyMsg.setString("No History Yet");
            subMsg.setString("Run an algorithm to see steps appear here");

            // Màu sắc: Trắng mờ (Alpha thấp để tạo cảm giác chìm vào nền)
            sf::Color ghostColor(255, 255, 255, 60);
            emptyIcon.setFillColor(ghostColor);
            emptyMsg.setFillColor(ghostColor);
            subMsg.setFillColor(sf::Color(255, 255, 255, 40));

            emptyIcon.setCharacterSize(60);
            emptyMsg.setCharacterSize(18);
            subMsg.setCharacterSize(14);

            // Căn giữa toàn bộ (Center Alignment)
            float centerX = m_width / 2.f;
            float centerY = m_height / 2.f - 20.f;

            auto centerText = [&](sf::Text& t, float y) {
                sf::FloatRect b = t.getLocalBounds();
                t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                t.setPosition(centerX, y);
            };

            centerText(emptyIcon, centerY - 40.f);
            centerText(emptyMsg, centerY + 20.f);
            centerText(subMsg, centerY + 45.f);

            target.draw(emptyIcon);
            target.draw(emptyMsg);
            target.draw(subMsg);
        }

        // 4. KHÔI PHỤC VIEW VÀ VẼ SCROLLBAR
        target.setView(originalView);
        target.draw(m_scrollbar, boardStates);

        // VẼ NHÁP ĐỂ KIỂM TRA:
//        sf::RectangleShape debugRect({10.f, m_height});
//        debugRect.setFillColor(sf::Color::Red);
//        debugRect.setPosition(m_width - 10.f, 0.f); // Vẽ một thanh màu đỏ sát lề phải Board
//        target.draw(debugRect, boardStates);
    }
}
