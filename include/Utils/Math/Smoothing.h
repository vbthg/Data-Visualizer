#pragma once
#include <cmath>
#include <SFML/Graphics.hpp>
#include <algorithm>

namespace Utils
{
    namespace Math
    {
        namespace Smoothing
        {
            // 1. Damp Position (Velocity Clamp - Chống rùa bò)
            template <typename T>
            inline T damp(T current, T target, float smoothing, float dt, float minSpeed = 0.1f)
            {
                float diff = target - current;
                float step = diff * (1.0f - std::pow(smoothing, dt));
                float minStep = minSpeed * dt;

                // Nếu bước đi quá nhỏ, cưỡng bức đi với tốc độ minSpeed
                if(std::abs(step) < minStep && std::abs(diff) > minStep)
                {
                    return current + ((diff > 0) ? minStep : -minStep);
                }
                if(std::abs(diff) < minStep)
                {
                    return target;
                }

                return current + step;
            }

            // 2. Damp Color (Velocity Clamp) - QUAN TRỌNG CHO MÀU NỀN
            inline sf::Color dampColor(sf::Color current, sf::Color target, float smoothing, float dt, float minSpeed = 10.0f)
            {
                auto dampChannel = [&](float c, float t) -> float
                {
                    float diff = t - c;
                    float step = diff * (1.0f - std::pow(smoothing, dt));
                    float minS = minSpeed * dt;

                    if(std::abs(diff) < minS) return t;
                    if(std::abs(step) < minS) return c + ((diff > 0) ? minS : -minS);
                    return c + step;
                };

                return sf::Color(
                    (sf::Uint8)dampChannel(current.r, target.r),
                    (sf::Uint8)dampChannel(current.g, target.g),
                    (sf::Uint8)dampChannel(current.b, target.b),
                    (sf::Uint8)dampChannel(current.a, target.a)
                );
            }

            // 3. Hàm Spring thô (Giữ lại nếu bạn muốn dùng thủ công)
            inline void spring(float& current, float target, float& velocity, float stiffness, float damping, float dt)
            {
                float displacement = current - target;
                float springForce = -stiffness * displacement;
                float dampForce = -damping * velocity;
                float acceleration = springForce + dampForce;
                velocity += acceleration * dt;
                current += velocity * dt;
            }
        }
    }
}
