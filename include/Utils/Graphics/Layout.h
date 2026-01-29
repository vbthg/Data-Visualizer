#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    namespace Graphics
    {
        // Hàm này sẽ tính toán Viewport để tạo viền đen nếu cần
        void maintainAspectRatio(sf::RenderWindow& window, sf::View& view);
    }
}
