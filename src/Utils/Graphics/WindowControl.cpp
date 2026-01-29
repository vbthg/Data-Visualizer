#include "WindowControl.h"
#include "WindowConfig.h" // Để lấy APP_TITLE
#include "Layout.h"

namespace Utils
{
    namespace Graphics
    {
        void centerWindow(sf::RenderWindow& window)
        {
            // Lấy độ phân giải màn hình desktop
            sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

            // Lấy kích thước cửa sổ hiện tại
            sf::Vector2u windowSize = window.getSize();

            // Tính toán tọa độ để căn giữa
            int x = (desktop.width - windowSize.x) / 2;
            int y = (desktop.height - windowSize.y) / 2;

            // Đặt vị trí
            window.setPosition(sf::Vector2i(x, y));
        }

        void toggleFullscreen(sf::RenderWindow& window, sf::View& view, bool& isFullscreen)
        {
            // 1. Đảo trạng thái logic
            isFullscreen = !isFullscreen;

            // 2. Lưu lại settings hiện tại (Antialiasing...) trước khi hủy cửa sổ
            sf::ContextSettings settings = window.getSettings();

            // 3. Tạo lại cửa sổ dựa trên trạng thái mới
            if(isFullscreen)
            {
                // Fullscreen: Lấy độ phân giải màn hình hiện tại của máy tính
                window.create(sf::VideoMode::getDesktopMode(),
                              System::APP_TITLE,
                              sf::Style::Fullscreen,
                              settings);
            }
            else
            {
                // Windowed: Trở về kích thước mặc định (ví dụ 1280x720)
                window.create(sf::VideoMode(1280, 720),
                              System::APP_TITLE,
                              sf::Style::Default,
                              settings);
            }

            // 4. QUAN TRỌNG: Cài đặt lại các thông số bị mất sau khi re-create
            window.setVerticalSyncEnabled(true);

            maintainAspectRatio(window, view);

            // (Lưu ý: Không cần setView ở đây, vì vòng lặp renderFrame trong main sẽ tự tính lại ngay lập tức)
        }
    }
}
