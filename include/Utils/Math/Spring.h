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
                // Gọi hàm thô từ namespace Smoothing
                Utils::Math::Smoothing::spring(position, target, velocity, stiffness, damping, dt);
            }

            void snapTo(float val)
            {
                position = val;
                target = val;
                velocity = 0.0f;
            }
        };
    }
}
