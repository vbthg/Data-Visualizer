#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

namespace GUI
{
    class Squircle : public sf::Drawable, public sf::Transformable
    {
    public:
        Squircle(const sf::Vector2f& size = {0, 0}, float radius = 0, float curvature = 4.0f)
        {
            mySize = size;
            myCurvature = curvature;
            // Mặc định bo 4 góc giống nhau
            setCornerRadius(radius, radius, radius, radius);
            update();
        }

        // Set kích thước
        void setSize(const sf::Vector2f& size)
        {
            mySize = size;
            update();
        }

        const sf::Vector2f& getSize() const
        {
            return mySize;
        }

        // Set màu
        void setFillColor(const sf::Color& color)
        {
            myShape.setFillColor(color);
        }

        const sf::Color& getFillColor() const
        {
            return myShape.getFillColor();
        }

        // Set bo từng góc: TopLeft, TopRight, BottomRight, BottomLeft
        void setCornerRadius(float tl, float tr, float br, float bl)
        {
            radii[0] = tl;
            radii[1] = tr;
            radii[2] = br;
            radii[3] = bl;
            update();
        }

        // Set bo đều 4 góc
        void setRadius(float radius)
        {
            setCornerRadius(radius, radius, radius, radius);
        }

        // Set độ cong Squircle (4.0 là chuẩn iOS)
        void setCurvature(float curvature)
        {
            myCurvature = curvature;
            update();
        }

        // Lấy bounds để check click
        sf::FloatRect getGlobalBounds() const
        {
            return getTransform().transformRect(myShape.getGlobalBounds());
        }

    private:
        sf::Vector2f mySize;
        float radii[4]; // 0:TL, 1:TR, 2:BR, 3:BL
        float myCurvature;
        sf::ConvexShape myShape;

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            target.draw(myShape, states);
        }

        // Hàm tính toán điểm Superellipse
        sf::Vector2f getSuperellipsePoint(float t, float n)
        {
            float cosT = std::cos(t);
            float sinT = std::sin(t);
            auto sign = [](float x) { return (x > 0) - (x < 0); };

            float x = std::pow(std::abs(cosT), 2.0f / n) * sign(cosT);
            float y = std::pow(std::abs(sinT), 2.0f / n) * sign(sinT);
            return {x, y};
        }

        void update()
        {
            // Nếu size quá nhỏ, không vẽ
            if (mySize.x <= 0 || mySize.y <= 0) return;

            // Số điểm cho mỗi góc bo (càng cao càng mượt)
            const int quality = 40;
            const float PI = 3.14159265f;

            myShape.setPointCount(quality * 4);
            int idx = 0;

            // Helper để giới hạn bán kính không quá 1 nửa cạnh
            auto getValidR = [&](float r)
            {
                return std::min(r, std::min(mySize.x, mySize.y) / 2.0f);
            };

            // 1. Bottom Right (Góc phần tư 1: 0 -> 90)
            float r = getValidR(radii[2]);
            sf::Vector2f centerBR(mySize.x - r, mySize.y - r);
            for (int i = 0; i < quality; ++i)
            {
                float t = 0.0f + (PI / 2.0f) * ((float)i / (quality - 1));
                sf::Vector2f p = getSuperellipsePoint(t, myCurvature);
                myShape.setPoint(idx++, centerBR + sf::Vector2f(p.x * r, p.y * r));
            }

            // 2. Bottom Left (90 -> 180)
            r = getValidR(radii[3]);
            sf::Vector2f centerBL(r, mySize.y - r);
            for (int i = 0; i < quality; ++i)
            {
                float t = PI / 2.0f + (PI / 2.0f) * ((float)i / (quality - 1));
                sf::Vector2f p = getSuperellipsePoint(t, myCurvature);
                myShape.setPoint(idx++, centerBL + sf::Vector2f(p.x * r, p.y * r));
            }

            // 3. Top Left (180 -> 270)
            r = getValidR(radii[0]);
            sf::Vector2f centerTL(r, r);
            for (int i = 0; i < quality; ++i)
            {
                float t = PI + (PI / 2.0f) * ((float)i / (quality - 1));
                sf::Vector2f p = getSuperellipsePoint(t, myCurvature);
                myShape.setPoint(idx++, centerTL + sf::Vector2f(p.x * r, p.y * r));
            }

            // 4. Top Right (270 -> 360)
            r = getValidR(radii[1]);
            sf::Vector2f centerTR(mySize.x - r, r);
            for (int i = 0; i < quality; ++i)
            {
                float t = 1.5f * PI + (PI / 2.0f) * ((float)i / (quality - 1));
                sf::Vector2f p = getSuperellipsePoint(t, myCurvature);
                myShape.setPoint(idx++, centerTR + sf::Vector2f(p.x * r, p.y * r));
            }
        }
    };
}
