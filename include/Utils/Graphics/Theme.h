#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Config.hpp>

namespace Utils
{
    namespace Graphics
    {
        namespace Theme
        {
            // --- 1. VẬT LÝ & CHUYỂN ĐỘNG (PHYSICS & TIMING) ---
            namespace Animation
            {
                // -- Physics (Vật lý lò xo) --
                constexpr float ButtonStiffness = 700.0f;
                constexpr float ButtonDamping = 40.0f;

                constexpr float CardStiffness = 300.0f;
                constexpr float CardDamping = 22.0f;

                // -- Sequence Timing (Animation 4 giai đoạn) --
                constexpr float Time_Feedback = 0.15f;
                constexpr float Scale_Press = 1.03f;

                constexpr float Time_HeroMove = 0.5f;
                constexpr float Time_AlgoEntry = 0.4f;

                // Duration cũ (Tương thích ngược)
                constexpr float CardAnimDuration = 0.6f;

                // -- Smoothing (Mượt mà màu sắc) --
                constexpr float ColorSmoothing = 0.05f;
                constexpr float ColorSnapSpeed = 1000.0f;
                constexpr float BgSmoothing = 0.2f;
                constexpr float BgMinSpeed = 100.0f;

                constexpr float CategoriesOutDuration = 0.25f;
                constexpr float HeroDuration = 0.25f;

                // --- MENU STATE IN-TRANSITION TIMING ---
                constexpr float MenuStaggerDelay = 0.08f;
                constexpr float MenuSlideDuration = 0.5f;
                constexpr float MenuSubtitleDuration = 0.6f;
                constexpr float MenuSlideDistance = 100.0f;
                constexpr float ShadowDuration = 0.4f;

                // --- VISUALIZER STATE & DOCK ANIMATION ---
                constexpr float HighlightStiffness = 350.0f; // Cập nhật cho Dock mới
                constexpr float HighlightDamping = 25.0f;    // Cập nhật cho Dock mới

//                constexpr float MorphStiffness = 200.0f;
//                constexpr float MorphDamping = 20.0f;

                constexpr float ShakeStiffness = 800.0f;
                constexpr float ShakeDamping = 15.0f;

                constexpr float SlideStiffness = 250.0f;
                constexpr float SlideDamping = 20.0f;

                constexpr float AlphaDampSpeed = 0.005f;


                // Vật lý cho sự co dãn chiều ngang của toàn bộ Dock
                inline const float DockWidthStiffness = 400.0f;
                inline const float DockWidthDamping = 30.0f;

                // Vật lý cho sự nở ra của SpeedController (Nhanh và nảy hơn chút)
                inline const float MorphStiffness = 500.0f;
                inline const float MorphDamping = 30.0f;

                inline const float HoldDelay = 0.5f; // Ngưỡng thời gian nhấn giữ để Expand
            }

            // --- 2. HÌNH DÁNG & KÍCH THƯỚC (GEOMETRY) ---
            namespace Style
            {
                // Squircle Curvature
                constexpr float SquircleCurvature = 4.0f;

                // -- Kích thước Bảng --
                // 1. Bảng Categories (960x460)
                constexpr float CatBoardW = 960.0f;
                constexpr float CatBoardH = 460.0f;
                constexpr float CatCardW = 320.0f;
                constexpr float CatCardH = 460.0f;
                constexpr float CatRadius = 48.0f;

                // 2. Bảng Data Structures (1200x580)
                constexpr float AlgoBoardW = 1200.0f;
                constexpr float AlgoBoardH = 580.0f;
                constexpr float AlgoCardW = 300.0f;
                constexpr float AlgoCardH = 580.0f;
                constexpr float AlgoRadius = 48.0f;

                // Item con
                constexpr float ItemRadius = 48.0f;

                // Radius Nút bấm
                constexpr float ButtonRadius = 12.0f;

                // -- Tương thích ngược --
                constexpr float MenuWidth = CatBoardW;
                constexpr float MenuHeight = CatBoardH;
                constexpr float CardRadius = CatRadius;

                // --- DOCK & POPOVER STYLE ---
                constexpr float DockHeight = 70.0f;       // Tăng lên 70 cho chuẩn thiết kế lưới
                constexpr float DockGridSize = 70.0f;     // Kích thước 1 ô chứa nút (Thay cho Padding/Gap cũ)
                constexpr float DockRadius = 55.0f;
                constexpr float DockMarginBottom = 16.0f;

                constexpr float DockHighlightRadius = 16.0f;
                constexpr float HighlightIdleWidth = 10.0f; // Bề ngang thu nhỏ khi không hover

                constexpr float IconButtonSize = 44.0f;
                constexpr float PopoverRadius = 24.0f;
                constexpr float InputRadius = 12.0f;

                // Cấu trúc mới cho Dock
                inline const float DockPadding = 16.0f;        // Lề trái/phải của Dock
                inline const float DockItemGap = 8.0f;         // Khoảng cách giữa các nút

                inline const float SeparatorWidth = 2.0f;      // Độ dày vạch ngăn
                inline const float SeparatorMargin = 16.0f;    // Khoảng không gian vạch ngăn chiếm (width + padding)

                inline const float SpeedControllerGap = 15.0f; // Khoảng cách giữa nút 1x và Slider khi nở ra
                inline const float SpeedSliderWidth = 120.0f;  // Chiều dài thanh trượt tốc độ
            }

            // --- 3. BẢNG MÀU (COLORS) ---
            namespace Color
            {
                // Nền ứng dụng
                const sf::Color Background = sf::Color(245, 245, 247);
                const sf::Color Shadow = sf::Color(0, 0, 0, 30);

                // -- Minimalist Style --
                const sf::Color CardSurface = sf::Color::White;
                const sf::Color TransparentColor = sf::Color(255, 255, 255, 0);
                const sf::Color CardSurfaceHover = sf::Color(252, 252, 254);

                // Màu chữ
                const sf::Color TextPrimary = sf::Color::Black;
                const sf::Color TextSecondary = sf::Color(120, 120, 120);

                // Màu Accent
                const sf::Color AccentPink = sf::Color(255, 105, 180);

                inline const sf::Color ButtonOutlineColor = sf::Color(0, 0, 0, 15);

                // --- UI CONTROLS COLORS ---
                inline const sf::Color Surface = sf::Color(255, 255, 255);
                inline const sf::Color InputBackground = sf::Color(245, 245, 245);
                inline const sf::Color PlaceholderText = sf::Color(150, 150, 150);

                inline const sf::Color Primary = sf::Color(0, 122, 255);
                inline const sf::Color Danger = sf::Color(255, 59, 48);

                // Dock Highlight Colors
                inline const float HighlightActiveAlpha = 20.0f;
                inline const float HighlightIdleAlpha = 0.0f;

                // Màu vạch ngăn (Trắng, Alpha 12% ~ 30/255)
                inline const sf::Color SeparatorTint = sf::Color(255, 255, 255, 30);
            }
        }
    }
}
