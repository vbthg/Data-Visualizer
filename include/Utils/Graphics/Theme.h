#pragma once
#include <SFML/Graphics/Color.hpp>

namespace Utils
{
    namespace Graphics
    {
        namespace Theme
        {
            // --- 1. VẬT LÝ & CHUYỂN ĐỘNG (PHYSICS) ---
            namespace Animation
            {
                // Button (Apple Style: Cứng, Đầm, Dứt khoát)
                constexpr float ButtonStiffness = 700.0f;
                constexpr float ButtonDamping = 40.0f;

                // Menu Card (Accordion: Mềm mại hơn 1 chút để lướt)
                constexpr float CardStiffness = 300.0f;
                constexpr float CardDamping = 22.0f;

                // --- MENU CARD (EASING) ---
                // Thời gian chuyển cảnh (giây).
                // 0.8f = Chậm rãi, điện ảnh
                // 0.6f = Nhanh, gọn
                constexpr float CardAnimDuration = 0.6f;

                // Tốc độ đổi màu (Color Smoothing)
                constexpr float ColorSmoothing = 0.08f;
                constexpr float ColorSnapSpeed = 500.0f; // Ngưỡng ngắt màu

                // Tốc độ đổi màu nền Background (Rất chậm & Mượt)
                constexpr float BgSmoothing = 0.2f;
                constexpr float BgMinSpeed = 10.0f;
            }

            // --- 2. HÌNH DÁNG & KÍCH THƯỚC (GEOMETRY) ---
            namespace Style
            {
                // Squircle Curvature (Độ cong siêu elip)
                // 4.0f = Chuẩn icon iOS/macOS (Rất đẹp)
                constexpr float SquircleCurvature = 4.0f;

                // Radius (Bán kính bo góc)
                constexpr float ButtonRadius = 12.0f; // Nút bấm
                constexpr float CardRadius = 48.0f;   // Bảng to (Container)
                constexpr float ItemRadius = 48.0f;   // Thẻ con bên trong

                // Kích thước Layout mặc định
                constexpr float MenuWidth = 1100.0f;
                constexpr float MenuHeight = 550.0f;
            }

            // --- 3. BẢNG MÀU (COLORS) ---
            namespace Color
            {
                // Nền ứng dụng (Xám nhạt clean)
                const sf::Color Background = sf::Color(245, 245, 247);

                // Bóng đổ (Shadow)
                const sf::Color Shadow = sf::Color(0, 0, 0, 30);

                // Màu chữ
                const sf::Color TextPrimary = sf::Color::Black;
                const sf::Color TextSecondary = sf::Color(120, 120, 120);

                // Màu Accent (Ví dụ: Màu hồng Juicy)
                const sf::Color AccentPink = sf::Color(255, 105, 180);
            }
        }
    }
}
