#include "TitleBar.h"
#include "Smoothing.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace GUI
{
    TitleBar::TitleBar(sf::RenderWindow& window, const sf::Font& iconFont)
        : windowRef(window),
          btnClose(iconFont, L"\uE8BB", {46.f, 32.f}),
          btnMaximize(iconFont, L"\uE922", {46.f, 32.f}),
          btnMinimize(iconFont, L"\uE921", {46.f, 32.f})
    {
        // --- SETUP STATE ---
        // Mặc định Fullscreen ngay từ đầu
        isFullscreen = true;
        currentY = -HEIGHT;
        targetY = -HEIGHT;
        velocityY = 0.0f;
        isVisible = false;

        // Dragging state
        isDragging = false;

        // Setup UI
        barBackground.setFillColor(sf::Color(255, 255, 255, 245));

        // Setup Buttons Styles
        btnClose.applyPreset(ButtonPreset::Danger);   btnClose.setCornerRadius(0); btnClose.setCharacterSize(10);
        btnMaximize.applyPreset(ButtonPreset::Ghost); btnMaximize.setCornerRadius(0); btnMaximize.setCharacterSize(10);
        btnMinimize.applyPreset(ButtonPreset::Ghost); btnMinimize.setCornerRadius(0); btnMinimize.setCharacterSize(10);

        // --- GÁN LOGIC NỘI BỘ (ENCAPSULATION) ---
        // TitleBar tự gọi hàm nội bộ của mình, Main không cần can thiệp

        btnClose.onClick = [this]() {
            windowRef.close();
        };

        btnMinimize.onClick = [this]() {
            minimize();
        };

        btnMaximize.onClick = [this]() {
            toggleFullscreen();
        };

        // Cập nhật icon ban đầu (vì đang Fullscreen nên icon phải là Restore)
        updateMaximizeIcon();
    }

    // --- HELPER FUNCTIONS ---

    void TitleBar::minimize() {
#ifdef _WIN32
        HWND hwnd = windowRef.getSystemHandle();
        ShowWindow(hwnd, SW_MINIMIZE);
#endif
    }

    void TitleBar::toggleFullscreen()
    {
        isFullscreen = !isFullscreen;

        if (isFullscreen)
        {
            // Lưu lại vị trí cũ để sau này khôi phục
            lastPos = windowRef.getPosition();
            lastSize = windowRef.getSize();

            // Chuyển sang Fullscreen
            windowRef.create(sf::VideoMode::getDesktopMode(), "Algorithm Visualizer", sf::Style::None);

            // Hiện TitleBar custom của ta lên
            isVisible = true;
        }
        else
        {
            // Quay về Windowed (Borderless)
            // Nếu chưa có vị trí cũ (lần đầu mở app), set mặc định 1600x900 giữa màn hình
            if (lastSize.x == 0)
            {
                lastSize = {1600, 900};
                lastPos = sf::Vector2i
                (
                    (sf::VideoMode::getDesktopMode().width - 1600) / 2,
                    (sf::VideoMode::getDesktopMode().height - 900) / 2
                );
            }
            windowRef.create(sf::VideoMode(lastSize.x, lastSize.y), "Algorithm Visualizer", sf::Style::Default);
            windowRef.setPosition(lastPos);
        }

        // Sau khi create lại cửa sổ, cần cài đặt lại FPS/Vsync
        windowRef.setFramerateLimit(60);
        windowRef.setVerticalSyncEnabled(true);

        // Cập nhật icon
        updateMaximizeIcon();
    }

    void TitleBar::updateMaximizeIcon()
    {
        if (isFullscreen) btnMaximize.setText(L"\uE923"); // 2 ô vuông
        else              btnMaximize.setText(L"\uE922"); // 1 ô vuông
    }

    // --- CORE LOOP ---

    void TitleBar::update(float dt)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(windowRef);
        sf::Vector2u winSize = windowRef.getSize();

        // 1. Logic Ẩn/Hiện
        // Nếu Fullscreen -> Chỉ hiện khi chuột lên mép trên
        // Nếu Windowed -> Luôn hiện (targetY = 0)

        if (!isFullscreen)
        {
            targetY = HEIGHT / 2.f;
            return;
        }
        else
        {
            bool mouseInZone = (mousePos.y < 5) || (mousePos.y < HEIGHT && isVisible);
            if (mouseInZone) {
                targetY = HEIGHT / 2.f;
                isVisible = true;
            } else {
                targetY = -HEIGHT / 2.f;
                isVisible = false;
            }
        }

        // Lò xo
        Utils::Math::Smoothing::spring(currentY, targetY, velocityY, 400.0f, 20.0f, dt);

        // 2. Layout
        barBackground.setPosition(0, currentY - HEIGHT / 2.f);
        barBackground.setSize(sf::Vector2f((float)winSize.x, HEIGHT));

        float startX = (float)winSize.x - 46.0f / 2.f;
        btnClose.setPosition({startX, currentY});         btnClose.update(windowRef, dt);
        btnMaximize.setPosition({startX - 46.f, currentY}); btnMaximize.update(windowRef, dt);
        btnMinimize.setPosition({startX - 92.f, currentY}); btnMinimize.update(windowRef, dt);
    }

    void TitleBar::handleEvent(const sf::Event& event)
    {
        // 1. XỬ LÝ KÉO CỬA SỔ (DRAGGING) - Chỉ làm khi Windowed
        if (!isFullscreen)
        {
            return;
//            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
//            {
//                sf::Vector2i mPos = sf::Mouse::getPosition(windowRef);
//                // Vùng kéo: Thanh bar (nhưng trừ khu vực nút bấm ra)
//                if (mPos.y < HEIGHT && mPos.x < windowRef.getSize().x - (46 * 3)) {
//                    isDragging = true;
//                    grabOffset = windowRef.getPosition() - sf::Mouse::getPosition();
//                }
//            }
//            else if (event.type == sf::Event::MouseButtonReleased) {
//                isDragging = false;
//            }
        }
        else {
            isDragging = false; // An toàn
        }

        // Di chuyển cửa sổ (nằm ngoài switch case event type)
        // Lưu ý: handleEvent thường chỉ xử lý event rời rạc, logic continuous nên để update
        // Nhưng logic drag đơn giản có thể để đây hoặc chuyển sang update nếu muốn mượt hơn.
        if (isDragging) {
            windowRef.setPosition(sf::Mouse::getPosition() + grabOffset);
        }

        // 2. XỬ LÝ NÚT BẤM
        if (currentY > -HEIGHT + 5.0f) {
            btnClose.handleEvent(event, windowRef);
            btnMaximize.handleEvent(event, windowRef);
            btnMinimize.handleEvent(event, windowRef);
        }
    }

    void TitleBar::draw()
    {
        // NẾU KHÔNG PHẢI FULLSCREEN -> KHÔNG VẼ
        if (!isFullscreen) return;

        if (currentY <= -HEIGHT) return;

        sf::View oldView = windowRef.getView();
        sf::View uiView(sf::FloatRect(0, 0, (float)windowRef.getSize().x, (float)windowRef.getSize().y));
        windowRef.setView(uiView);

        windowRef.draw(barBackground);
        btnMinimize.draw(windowRef);
        btnMaximize.draw(windowRef);
        btnClose.draw(windowRef);

        windowRef.setView(oldView);
    }
}
