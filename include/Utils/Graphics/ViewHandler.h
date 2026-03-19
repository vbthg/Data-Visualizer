#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    class ViewHandler
    {
    public:
        // Chuyển đổi từ tọa độ chuột trên Window sang tọa độ World bên trong một Frame (như khung CTDL)
        static sf::Vector2f mapPixelToWorld(sf::Vector2i mousePos, sf::RenderWindow& window, const sf::FloatRect& frameBounds, const sf::View& cameraView)
        {
            // 1. Chuyển từ pixel màn hình sang tọa độ tương đối trong Window (xử lý việc resize cửa sổ)
            sf::Vector2f windowPos = window.mapPixelToCoords(mousePos);

            // 2. Chuyển về tọa độ Local của Frame (trừ đi lề 24px)
            sf::Vector2f localPos = windowPos - sf::Vector2f(frameBounds.left, frameBounds.top);

            // 3. Sử dụng một RenderTarget tạm thời hoặc tính toán thủ công để ánh xạ vào View của Camera
            // Cách đơn giản nhất: Tính tỉ lệ (ratio) dựa trên kích thước View và kích thước Frame
            sf::Vector2f viewSize = cameraView.getSize();
            sf::Vector2f viewCenter = cameraView.getCenter();

            float ratioX = localPos.x / frameBounds.width;
            float ratioY = localPos.y / frameBounds.height;

            sf::Vector2f worldPos;
            worldPos.x = (viewCenter.x - viewSize.x / 2.0f) + (ratioX * viewSize.x);
            worldPos.y = (viewCenter.y - viewSize.y / 2.0f) + (ratioY * viewSize.y);

            return worldPos;
        }

        // Kiểm tra chuột có nằm trong một vùng Squircle/Frame không
        static bool isMouseInFrame(sf::Vector2i mousePos, sf::RenderWindow& window, const sf::FloatRect& frameBounds)
        {
            sf::Vector2f coords = window.mapPixelToCoords(mousePos);
            return frameBounds.contains(coords);
        }
    };
}
