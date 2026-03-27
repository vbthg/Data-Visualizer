#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    class ViewHandler
    {
    public:
        // Chuyển đổi từ tọa độ chuột trên Window sang tọa độ World bên trong một Frame (như khung CTDL)
        // ViewHandler.h
        static sf::Vector2f mapPixelToWorld(sf::Vector2i mousePos, const sf::RenderWindow& window, const sf::FloatRect& frameBounds, const sf::View& cameraView)
        {
            // 1. Chuyển từ pixel màn hình sang tọa độ UI trong Window
            sf::Vector2f windowCoords = window.mapPixelToCoords(mousePos);

            // 2. Tính tọa độ tương đối (0.0 -> 1.0) bên trong Frame
            // Sử dụng toán tử vector để tối ưu phép tính
            sf::Vector2f localNormalized;
            localNormalized.x = (windowCoords.x - frameBounds.left) / frameBounds.width;
            localNormalized.y = (windowCoords.y - frameBounds.top) / frameBounds.height;

            // 3. Nội suy ra tọa độ World dựa trên View hiện tại
            sf::Vector2f viewSize = cameraView.getSize();
            sf::Vector2f viewTopLeft = cameraView.getCenter() - viewSize / 2.0f;

            return viewTopLeft + sf::Vector2f(localNormalized.x * viewSize.x, localNormalized.y * viewSize.y);
        }

        // Kiểm tra chuột có nằm trong một vùng Squircle/Frame không
        static bool isMouseInFrame(sf::Vector2i mousePos, const sf::RenderWindow& window, const sf::FloatRect& frameBounds)
        {
            sf::Vector2f coords = window.mapPixelToCoords(mousePos);
            return frameBounds.contains(coords);
        }
    };
}
