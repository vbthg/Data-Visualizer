#pragma once
#include <cmath>
#include <SFML/Graphics.hpp>

const float PI = std::acos(-1);

namespace Utils
{
    namespace Math
    {
        namespace Easing
        {
            // 2. Ease In Cubic (Chậm -> Nhanh dần)
            inline float easeInCubic(float t)
            {
                return t * t * t;
            }

            // 1. Ease Out Cubic (Nhanh, mượt vừa phải - Standard iOS)
            inline float easeOutCubic(float t)
            {
                return 1.0f - std::pow(1.0f - t, 3.0f);
            }

            // 3. Ease Out Quart (Bậc 4 - CHUẨN MACOS MỚI)
            // Mềm hơn Quint (bậc 5) nhưng vẫn dứt khoát hơn Cubic.
            // Kết hợp với duration > 400ms là cực đẹp.
            inline float easeOutQuart(float t)
            {
                return 1.0f - std::pow(1.0f - t, 4.0f);
            }

            // 2. Ease Out Quint (Nhanh, dứt khoát hơn - MacOS Style) -> KHUYÊN DÙNG
            inline float easeOutQuint(float t)
            {
                return 1.0f - std::pow(1.0f - t, 5.0f);
            }

            // Ease Out Expo: Cực nhanh lúc đầu, cực chậm lúc sau.
            // Tạo cảm giác "hạ cánh" rất mềm.
            inline float easeOutExpo(float t)
            {
                return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
            }

            // --- BỘ HÀM SINE (MỚI) ---

            // 2. Ease Out Sine (Nhanh -> Chậm dần, nhưng rất mềm)
            inline float easeOutSine(float t)
            {
                return std::sin((t * PI) / 2.0f);
            }

            // 3. Ease In Sine (Chậm -> Nhanh dần, rất mềm)
            inline float easeInSine(float t)
            {
                return 1.0f - std::cos((t * PI) / 2.0f);
            }

            // 3. Hàm Lerp (Linear Interpolation) - Để tính giá trị giữa điểm đầu và cuối
            template <typename T>
            inline T lerp(const T& start, const T& end, float t)
            {
                return start + (end - start) * t;
            }

            // Lerp riêng cho SFML Color
            inline sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t)
            {
                return sf::Color(
                    (sf::Uint8)(start.r + (end.r - start.r) * t),
                    (sf::Uint8)(start.g + (end.g - start.g) * t),
                    (sf::Uint8)(start.b + (end.b - start.b) * t),
                    (sf::Uint8)(start.a + (end.a - start.a) * t)
                );
            }

        } // namespace Easing
    } // namespace Math
} // namespace Utils
