#include "Layout.h"
#include "WindowConfig.h" // Giả định bạn đã add search path

namespace Utils
{
    namespace Graphics
    {
        void maintainAspectRatio(sf::RenderWindow& window, sf::View& view)
        {
            // Tính tỉ lệ khung hình hiện tại
            float windowRatio = (float)window.getSize().x / (float)window.getSize().y;
            float designRatio = System::DESIGN_WIDTH / System::DESIGN_HEIGHT;
            float scale = 1.0f;

            // Logic Letterbox / Pillarbox
            if(windowRatio > designRatio)
            {
                // Màn hình quá bè ngang -> Viền đen trái/phải
                scale = designRatio / windowRatio;
                view.setViewport(sf::FloatRect((1.0f - scale) / 2.0f, 0.0f, scale, 1.0f));
            }
            else
            {
                // Màn hình quá cao -> Viền đen trên/dưới
                scale = windowRatio / designRatio;
                view.setViewport(sf::FloatRect(0.0f, (1.0f - scale) / 2.0f, 1.0f, scale));
            }

            // Luôn reset kích thước logic về 1920x1080
            view.setSize(System::DESIGN_WIDTH, System::DESIGN_HEIGHT);
            view.setCenter(System::DESIGN_WIDTH / 2.0f, System::DESIGN_HEIGHT / 2.0f);

            // Áp dụng ngay lập tức
            window.setView(view);
        }
    }
}
