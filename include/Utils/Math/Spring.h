#pragma once
#include "Smoothing.h"

namespace Utils
{
    namespace Physics
    {
        // Wrapper class để dễ dùng trong Button
        struct Spring
        {
            float position = 1.0f;
            float target = 1.0f;
            float velocity = 0.0f;

            // Cấu hình chuẩn Apple (Đầm - Chắc - Không rung lắc)
            float stiffness = 600.0f; // Cứng (phản hồi nhanh)
            float damping = 35.0f;    // Ma sát cao (để không bị nảy tưng tưng)

            void update(float dt)
            {
                // Tạo một biến local mới chỉ dùng cho công thức lò xo
                float physicsDt = std::min(dt, 0.0166f);
                // Gọi hàm thô từ namespace Smoothing
                Utils::Math::Smoothing::spring(position, target, velocity, stiffness, damping, physicsDt);
            }

            void snapTo(float val)
            {
                position = val;
                target = val;
                velocity = 0.0f;
            }
        };

        struct Spring2D
        {
            sf::Vector2f position = {1.0f, 1.0f};
            sf::Vector2f target = {1.0f, 1.0f};
            sf::Vector2f velocity = {0.0f, 0.0f};

            float stiffness = 600.0f;
            float damping = 35.0f;

            void update(float dt)
            {
                float physicsDt = std::min(dt, 0.0166f);
                Utils::Math::Smoothing::spring(position.x, target.x, velocity.x, stiffness, damping, physicsDt);
                Utils::Math::Smoothing::spring(position.y, target.y, velocity.y, stiffness, damping, physicsDt);
            }

            void snapTo(sf::Vector2f val)
            {
                position = val;
                target = val;
                velocity = {0.0f, 0.0f};
            }
        };
    }
}
