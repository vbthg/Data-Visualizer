#pragma once
#include <cmath>
#include <algorithm>
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
            inline float easeOutQuart(float t)
            {
                return 1.0f - std::pow(1.0f - t, 4.0f);
            }

            // 2. Ease Out Quint (Nhanh, dứt khoát hơn - MacOS Style)
            inline float easeOutQuint(float t)
            {
                return 1.0f - std::pow(1.0f - t, 5.0f);
            }

            // Ease Out Expo
            inline float easeOutExpo(float t)
            {
                return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
            }

            // --- BỘ HÀM BACK (MỚI THÊM) ---
            inline float easeOutBack(float t)
            {
                const float c1 = 1.70158f;
                const float c3 = c1 + 1.0f;
                return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
            }

            // --- BỘ HÀM SINE ---
            inline float easeOutSine(float t)
            {
                return std::sin((t * PI) / 2.0f);
            }

            inline float easeInSine(float t)
            {
                return 1.0f - std::cos((t * PI) / 2.0f);
            }

            // --- LERP ---
            template <typename T>
            inline T lerp(const T& start, const T& end, float t)
            {
                return start + (end - start) * t;
            }

            inline sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t)
            {
                auto safeLerpChannel = [](float start, float end, float t) -> sf::Uint8
                {
                    float val = start + (end - start) * t;
                    if (val < 0.0f) return 0;
                    if (val > 255.0f) return 255; // Quan trọng: Khóa trần ở 255 thay vì bị tràn số
                    return (sf::Uint8)val;
                };

                return sf::Color(
                    safeLerpChannel(start.r, end.r, t),
                    safeLerpChannel(start.g, end.g, t),
                    safeLerpChannel(start.b, end.b, t),
                    safeLerpChannel(start.a, end.a, t)
//                    (sf::Uint8)(start.r + (end.r - start.r) * t),
//                    (sf::Uint8)(start.g + (end.g - start.g) * t),
//                    (sf::Uint8)(start.b + (end.b - start.b) * t),
//                    (sf::Uint8)max(255.f, (float(start.a) + float(end.a - start.a) * t)
                );
            }

        } // namespace Easing
    } // namespace Math
} // namespace Utils
