#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Squircle.h"

// Utils.h hoặc một file chuyên biệt cho Graphics
namespace Utils
{
    namespace Graphics
    {
        // Hàm vẽ một tấm kính mờ (Glass Pane)
        inline void drawGlassPane(sf::RenderWindow& window,
                         GUI::Squircle& frame,
                         const sf::Texture& blurBackground,
                         const sf::Color& overlayColor,
                         const sf::Texture* contentTexture = nullptr)
        {
            // Lấy tọa độ thực tế của khung
            sf::FloatRect bounds = frame.getGlobalBounds();

            // --- LỚP 1: VẼ KÍNH MỜ (BẮT BUỘC PHẢI HIỆN RÕ 100%) ---
            frame.setTexture(&blurBackground);
            frame.setTextureRect(sf::IntRect((int)bounds.left, (int)bounds.top, (int)bounds.width, (int)bounds.height));

            // Đặt màu Trắng với Alpha 255 để Texture mờ không bị trong suốt
            frame.setFillColor(sf::Color::White);
            window.draw(frame);

            // --- LỚP 2: VẼ LỚP PHỦ TẠO ĐỘ ĐỤC (OVERLAY TINT) ---
            frame.setTexture(nullptr); // Gỡ texture ra để chỉ vẽ màu trơn
            frame.setFillColor(overlayColor); // Áp dụng màu phủ (VD: sf::Color(255, 255, 255, 20))
            window.draw(frame);

            // --- LỚP 3: VẼ NỘI DUNG (NẾU CÓ) ---
            if(contentTexture)
            {
                sf::Sprite contentSprite(*contentTexture);
                contentSprite.setPosition(bounds.left, bounds.top);
                window.draw(contentSprite);
            }
        }
    }
}
