#include "GUI/NotchManager.h"
#include "GUI/NotchContent.h"
#include "FileTrayContent.h"
#include "ViewHandler.h"
#include "InputContent.h"
#include "FileDropManager.h"
#include "ResourceManager.h"
#include "ProgressBar.h"
#include <iostream>

namespace GUI
{
    NotchManager::NotchManager()
        : m_currentScenario(Scenario::Idle),
          m_currentSizeState(NotchSize::Compact),
          m_dismissTimer(0.f),
          m_dismissTargetTime(0.f),
          m_isAutoDismissing(false),
          m_lastTitle(""), // Khởi tạo ở đây
          m_lastSubtitle(""),
          m_lastStep(-1) // Khởi tạo giá trị không tưởng
    {
    }

    void NotchManager::init(sf::Vector2f screenSize)
    {
        m_screenSize = screenSize;
        m_notch = std::make_unique<GUI::Notch>(SIZE_COMPACT);
        m_notch->setRadii(RADII_COMPACT.x, RADII_COMPACT.y);
        m_notch->setFillColor(sf::Color::Black);
        m_notch->setShadow(sf::Color(0, 0, 0, 150), 20.f, sf::Vector2f(0.f, 10.f));
        m_notch->setPosition(m_screenSize.x / 2.f, 0.f);

        applySize(NotchSize::Compact);
    }

    sf::FloatRect GUI::NotchManager::getBounds() const
    {
        if(m_notch)
        {
            sf::FloatRect bounds = m_notch->getGlobalBounds();
            bounds.left -= 30.f;
            bounds.width += 60.f;
            bounds.height += 30.f;
            return bounds;
        }
        return sf::FloatRect();
    }

    void GUI::NotchManager::updateMousePos(sf::Vector2i mousePos, const sf::RenderWindow& window)
    {
        if(!m_notch) return;

        bool isHovered = Utils::ViewHandler::isMouseInFrame(mousePos, window, getBounds());

//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
//        { // Nhấn Space để in log tránh spam
//            std::cout << "Mouse UI Coords: " << mousePos.x << ", " << mousePos.y << std::endl;
//            std::cout << "Notch Bounds: L:" << getBounds().left << " T:" << getBounds().top
//                      << " W:" << getBounds().width << " H:" << getBounds().height << std::endl;
//        }

        if(m_currentScenario == Scenario::Initial || m_currentScenario == Scenario::FileTray)
        {
            if(isHovered)
            {
                 m_notch->setScaleTarget(1.03f);
            }
            else
            {
                 m_notch->setScaleTarget(1.0f);
            }
        }
    }

    void NotchManager::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        // 1. Chuyển tiếp sự kiện cho Content (Quan trọng để InputContent gõ phím)
        if(m_notch && m_notch->getContent())
        {
            m_notch->getContent()->handleEvent(event, window);
        }

        // 2. Logic Double Click để mở Input
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i pixelPos(event.mouseButton.x, event.mouseButton.y);

            // Dùng hàm cũ của em để kiểm tra tọa độ logic
            if(Utils::ViewHandler::isMouseInFrame(pixelPos, window, getBounds()))
            {
                float elapsed = m_clickClock.getElapsedTime().asSeconds();

                if(elapsed < DOUBLE_CLICK_THRESHOLD)
                {
                    // Nhận diện Double Click thành công!
                    if(m_currentScenario == Scenario::Idle || m_currentScenario == Scenario::Success)
                    {
                        pushNotification(Scenario::Input);
                    }
                }

                m_clickClock.restart(); // Reset đồng hồ sau mỗi lần click
            }
        }
    }

    void NotchManager::applySize(NotchSize targetSize)
    {
        m_currentSizeState = targetSize;
        switch(targetSize)
        {
            case NotchSize::Compact:
                m_notch->setSize(SIZE_COMPACT);
                m_notch->setRadii(RADII_COMPACT.x, RADII_COMPACT.y);
                break;
            case NotchSize::Standard:
                m_notch->setSize(SIZE_STANDARD);
                m_notch->setRadii(RADII_STANDARD.x, RADII_STANDARD.y);
                break;
            case NotchSize::Expanded:
                m_notch->setSize(SIZE_EXPANDED);
                m_notch->setRadii(RADII_EXPANDED.x, RADII_EXPANDED.y);
                break;
            case NotchSize::Tray:
                m_notch->setSize(SIZE_TRAY);
                m_notch->setRadii(RADII_TRAY.x, RADII_TRAY.y);
                break;
            case NotchSize::Input:
                m_notch->setSize(SIZE_INPUT);
                m_notch->setRadii(RADII_INPUT.x, RADII_INPUT.y);
        }
    }

    void NotchManager::pushNotification(Scenario type, const sf::String& title, const sf::String& subtitle, const sf::String& iconCode)
    {
//        std::cout << "[DEBUG] Push: " << (int)type << " | Title: " << title << " " << title.size() << "\n";

    // Dòng debug cực kỳ quan trọng
//    std::cout << "[NOTCH DEBUG] Pushing Scenario: " << (int)type << " | Title: " << title.toAnsiString() << std::endl;

//        if(m_currentScenario == Scenario::FileTray && type != Scenario::FileTray)
//        {
//            GUI::FileTrayContent* currentTray = dynamic_cast<GUI::FileTrayContent*>(m_notch->getContent());
//            if(currentTray)
//            {
//                currentTray->setContentScale(0.f);
//            }
//            // Cần thêm logic delay việc changeContent() ở hàm update() cho đến khi scaleSpring chạm 0
////            return;
//        }
        m_currentScenario = type;

//        if(type == Scenario::FileTray) std::cout << "start PushNoti: title = " << title << "\n";

        NotchSize targetSize = NotchSize::Compact;
        m_isAutoDismissing = false;
        m_dismissTimer = 0.f;

        switch(type)
        {
            case Scenario::Initial:
            case Scenario::Idle:
            case Scenario::FileTray:
            case Scenario::Input:
            case Scenario::AwaitingInput:
                m_isAutoDismissing = false; // CÁC TRẠNG THÁI NÀY KHÔNG ĐƯỢC TỰ ẨN
                break;

            case Scenario::Success:
                m_dismissTargetTime = 2.0f; // Hiện 2 giây rồi ẩn
                break;

            case Scenario::Error:
                m_dismissTargetTime = 4.0f; // Lỗi thì hiện lâu hơn để đọc
                break;

            default:
                m_dismissTargetTime = 3.0f;
                break;
        }


        switch(type)
        {
            case Scenario::Initial:
                targetSize = NotchSize::Standard;
                break;
            case Scenario::Idle:
                targetSize = NotchSize::Compact;
                break;
            case Scenario::Processing:
                targetSize = NotchSize::Standard;
                break;
            case Scenario::Success:
                targetSize = NotchSize::Expanded;
                m_isAutoDismissing = true;
                m_dismissTargetTime = 2.5f;
                break;
            case Scenario::Error:
            case Scenario::Warning:
                targetSize = NotchSize::Expanded;
                m_isAutoDismissing = true;
                m_dismissTargetTime = 3.5f;
                break;
            case Scenario::AwaitingInput:
                targetSize = NotchSize::Expanded;
                break;
            case Scenario::FileTray:
                targetSize = NotchSize::Tray;
                break;
            case Scenario::Input:
                targetSize = NotchSize::Input;
                break;
        }

        applySize(targetSize);

        // --- ĐIỀU KHIỂN NHỊP ĐẬP WAVEFORM ---
        if(type == Scenario::Processing)
        {
            m_notch->setWaveformState(GUI::Waveform::State::Processing);
        }
        else if(type == Scenario::Success || type == Scenario::Error)
        {
            m_notch->setWaveformState(GUI::Waveform::State::Action);
        }
        else
        {
            m_notch->setWaveformState(GUI::Waveform::State::Idle);
        }
        // ------------------------------------

        m_notch -> setScenario(type);

        if(type == Scenario::Input)
        {
            auto content = std::make_unique<GUI::InputContent>(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));

            // Khi người dùng nhấn Enter sau khi gõ đường dẫn
            content->setOnSubmit([this](std::string path)
            {
                // Kiểm tra xem đã có "dây nối" callback chưa
                if(m_onFileDroppedCallback)
                {
                    // Gọi callback để bắt đầu quá trình nạp file thực sự
                    // Bên trong hàm loadFromFile của Structure sẽ tự lo việc hiện Processing/Success
                    m_onFileDroppedCallback(path);
                }
            });

            // Khi người dùng nhấn Escape để hủy việc nhập liệu
            content->setOnCancel([this]()
            {
                // Quay về trạng thái Idle (Viên thuốc tối giản)
                this->pushNotification(Scenario::Idle);
            });

            m_notch->changeContent(std::move(content));
        }
        else if(type == Scenario::FileTray)
        {
            // 1. Tạo Content mới (Dùng font từ ResourceManager)
            auto content = std::make_unique<GUI::FileTrayContent>(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));

            // 2. Cài đặt Callback: Khi có file thả vào -> Chuyển sang Processing
            content->setOnFileDropped([this](std::string path) {
                // Kiểm tra xem file có đúng định dạng không nếu muốn
                this->pushNotification(Scenario::Processing, "Importing Data", path, "\xef\x84\x9e");

                // Chạy thanh bar ảo (Cách 1 đã thống nhất)
//                this->m_notch->getProgressBar().setStep(1, 1, 1.5f);
            });

            // 3. Đẩy vào Notch để bắt đầu Morphing
            m_notch->changeContent(std::move(content));
            return; // Kết thúc hàm ở đây
        }
        else if(type == Scenario::Initial)
        {
            // Dùng icon dấu + (mã L"\xea\x8a" nếu đổi sang sf::String)
//            std::cout << "Made a Initial noti here!\n";
            auto content = std::make_unique<GUI::NotchContent>("", "Import Data", "Click to load file");
            m_notch->changeContent(std::move(content));
        }
        else if(type != Scenario::Idle)
        {
            auto content = std::make_unique<GUI::NotchContent>(iconCode, title, subtitle);
            m_notch->changeContent(std::move(content));
        }
        else
        {
            auto content = std::make_unique<GUI::NotchContent>("", "", "");
            m_notch->changeContent(std::move(content));
        }
    }

    void NotchManager::updateFromContext(const Core::NotchContext& ctx, float playbackSpeed, float cursor, int macroStartIdx)
    {
        if(m_currentScenario != ctx.scenario || m_lastTitle != ctx.title || m_lastSubtitle != ctx.subtitle)
        {
            this->pushNotification(ctx.scenario, ctx.title, ctx.subtitle, ctx.iconCode);
            m_lastTitle = ctx.title;
            m_lastSubtitle = ctx.subtitle;
        }

//        if(m_currentScenario == Scenario::Processing)
//        {
//            // Số quãng đường nối giữa N điểm là N - 1
//            int totalSegments = ctx.total - 1;
//
//            if(totalSegments > 0)
//            {
//                // Tính p dựa trên độ lệch cursor so với điểm bắt đầu macro
//                float p = (cursor - static_cast<float>(macroStartIdx)) / static_cast<float>(totalSegments);
//
//                // Cập nhật số lượng segment cho ProgressBar
//                m_notch->setStepInfo(0, totalSegments, 1.0f);
//                m_notch->getProgressBar().setPercent(p);
//            }
//            else
//            {
//                m_notch->getProgressBar().setPercent(1.0f);
//            }
//        }
    }

void NotchManager::update(float dt, sf::RenderWindow& window)
{
    // 1. ĐỒNG BỘ VIEW ĐỂ MAP TỌA ĐỘ (Xử lý Letterboxing)
//    sf::View oldView = window.getView();
//    window.setView(uiView);

    // 2. XỬ LÝ TỌA ĐỘ CHUỘT
    sf::Vector2i pixelPos;
    if(FileDropManager::isDragging())
    {
        sf::Vector2f hover = FileDropManager::getHoverPosition();
        pixelPos = sf::Vector2i(static_cast<int>(hover.x), static_cast<int>(hover.y));
    }
    else
    {
        pixelPos = sf::Mouse::getPosition(window);
    }

    // 3. KIỂM TRA HOVER (Dùng hàm cũ của em trong ViewHandler)
    sf::FloatRect notchBounds = getBounds();
    bool isHoveringNotch = Utils::ViewHandler::isMouseInFrame(pixelPos, window, notchBounds);
    bool currentlyDragging = FileDropManager::isDragging();

    // 1. XỬ LÝ DRAG & DROP (Vẫn là ưu tiên số 1)
    if (currentlyDragging)
    {
        if (m_currentScenario != Scenario::FileTray)
        {
            pushNotification(Scenario::FileTray, "Import Data", "Drop file to load", "\xef\x84\x9e");
        }

        auto tray = dynamic_cast<FileTrayContent*>(m_notch->getContent());
        if (tray) tray->setHoverState(isHoveringNotch);
    }
    // 2. XỬ LÝ KHI THẢ FILE
    else if (FileDropManager::hasDroppedFiles())
    {
        auto files = FileDropManager::popDroppedFiles();
        if (isHoveringNotch && !files.empty())
        {
            if (m_onFileDroppedCallback) m_onFileDroppedCallback(files[0]);
        }
        else
        {
            // Nếu thoát khỏi dragging mà không có file dropped trúng đích
            pushNotification(Scenario::Idle, "Import Data", "Drop file or double click here");
        }
    }
    // 3. LOGIC HỦY DRAG
    else if (m_currentScenario == Scenario::FileTray)
    {
        pushNotification(Scenario::Idle, "Import Data", "Drop file or double click here");
    }

    // 4. HIỆU ỨNG HOVER CHO IDLE (Quan trọng để gợi ý tương tác)
    if (!currentlyDragging)
    {
        if (m_currentScenario == Scenario::Idle)
        {
            // Khi hover vào viên thuốc, nó sẽ nở nhẹ ra và có thể đổi màu viền nhẹ
            m_notch->setScaleTarget(isHoveringNotch ? 1.05f : 1.0f);
        }
        else
        {
            m_notch->setScaleTarget(1.0f);
        }
    }

    // 6. LOGIC AUTO-DISMISS (Logic cũ của em: Success/Error tự ẩn sau x giây)
    if(m_isAutoDismissing)
    {
        m_dismissTimer += dt;
        if(m_dismissTimer >= m_dismissTargetTime)
        {
            // Hết giờ thì quay về trạng thái nghỉ (Idle)
            pushNotification(Scenario::Idle);
        }
    }

    // 7. LOGIC CHUYỂN CẢNH TỰ ĐỘNG (Ví dụ: Processing xong thì sang Success)
    // Đoạn này trong code cũ của em đang bị comment, anh mở lại nếu em cần:
//    if(m_currentScenario == Scenario::Processing)
//    {
//        if(m_notch->getProgressBar().isFinished())
//        {
//            pushNotification(Scenario::Success, "Imported", "Ready to visualize", "\xef\x80\x8c");
//        }
//    }

//    // 8. HIỆU ỨNG SCALE HOVER (Chỉ giãn ra khi ở trạng thái chờ)
//    if(!FileDropManager::isDragging())
//    {
//        if(m_currentScenario == Scenario::Initial || m_currentScenario == Scenario::Idle)
//        {
//            m_notch->setScaleTarget(isHoveringNotch ? 1.03f : 1.0f);
//        }
//        else
//        {
//            m_notch->setScaleTarget(1.0f); // Reset scale nếu đang ở scenario khác
//        }
//    }

    // 9. CẬP NHẬT VẬT LÝ LÒ XO CỦA NOTCH CORE
    if(m_notch)
    {
        m_notch->update(dt);
    }

    // 10. TRẢ LẠI VIEW CŨ
//    window.setView(oldView);
}

    void NotchManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if(m_notch)
        {
            target.draw(*m_notch, states);
        }
    }
}
