#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    namespace Graphics
    {
        // Hàm căn giữa cửa sổ
        void centerWindow(sf::RenderWindow& window);

        // Hàm này nhận vào biến tham chiếu isFullscreen để đảo trạng thái
        void toggleFullscreen(sf::RenderWindow& window, sf::View& view, bool& isFullscreen);
    }
}
