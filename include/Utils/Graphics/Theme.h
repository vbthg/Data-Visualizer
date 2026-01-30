#pragma once
#include <SFML/Graphics/Color.hpp>

namespace Utils
{
    namespace Graphics
    {
        namespace Theme
        {
            // --- 1. VẬT LÝ & CHUYỂN ĐỘNG (PHYSICS & TIMING) ---
            namespace Animation
            {
                // -- Physics (Vật lý lò xo - Giữ nguyên) --
                constexpr float ButtonStiffness = 700.0f;
                constexpr float ButtonDamping = 40.0f;

                constexpr float CardStiffness = 300.0f;
                constexpr float CardDamping = 22.0f;

                // -- Sequence Timing (Mới: Cho Animation 4 giai đoạn) --
                constexpr float Time_Feedback = 0.15f; // Giai đoạn 1: Click & Scale nhẹ
                constexpr float Scale_Press = 1.03f;   // Độ scale khi nhấn

                constexpr float Time_HeroMove = 0.5f;  // Giai đoạn 2: Hero bay
                constexpr float Time_AlgoEntry = 0.4f; // Giai đoạn 3: Bảng Algo trượt lên

                // Duration cũ (Giữ lại để tương thích code cũ nếu còn sót)
                constexpr float CardAnimDuration = 0.6f;

                // -- Smoothing (Mượt mà màu sắc) --
                constexpr float ColorSmoothing = 0.08f;
                constexpr float ColorSnapSpeed = 500.0f;
                constexpr float BgSmoothing = 0.2f;
                constexpr float BgMinSpeed = 10.0f;
            }

            // --- 2. HÌNH DÁNG & KÍCH THƯỚC (GEOMETRY) ---
            namespace Style
            {
                // Squircle Curvature
                constexpr float SquircleCurvature = 4.0f;

                // -- Kích thước Bảng Mới (Phân tách rõ ràng) --

                // 1. Bảng Categories (960x460)
                constexpr float CatBoardW = 960.0f;
                constexpr float CatBoardH = 460.0f;
                constexpr float CatRadius = 48.0f;

                // 2. Bảng Data Structures (1200x580)
                constexpr float AlgoBoardW = 1200.0f;
                constexpr float AlgoBoardH = 580.0f;
                constexpr float AlgoRadius = 48.0f;

                // Item con (Thẻ con bên trong bảng)
                constexpr float ItemRadius = 48.0f;

                // Radius Nút bấm
                constexpr float ButtonRadius = 12.0f;

                // -- Tương thích ngược (Backward Compatibility) --
                // Map các tên biến cũ vào giá trị mới để tránh lỗi biên dịch
                constexpr float MenuWidth = CatBoardW;
                constexpr float MenuHeight = CatBoardH;
                constexpr float CardRadius = CatRadius;
            }

            // --- 3. BẢNG MÀU (COLORS) ---
            namespace Color
            {
                // Nền ứng dụng
                const sf::Color Background = sf::Color(245, 245, 247);
                const sf::Color Shadow = sf::Color(0, 0, 0, 30); // Bóng 30% (Chuẩn cũ)

                // -- Minimalist Style (Mới) --
                const sf::Color CardSurface = sf::Color::White;
                const sf::Color CardSurfaceHover = sf::Color(252, 252, 254);

                // Màu chữ
                const sf::Color TextPrimary = sf::Color::Black;
                const sf::Color TextSecondary = sf::Color(120, 120, 120);

                // Màu Accent
                const sf::Color AccentPink = sf::Color(255, 105, 180);
            }
        }
    }
}
