#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

namespace GUI {

class RoundedRectangleShape : public sf::Drawable, public sf::Transformable {
public:
    RoundedRectangleShape(const sf::Vector2f& size = {0, 0}, float radius = 0) {
        mySize = size;
        // Mặc định bo 4 góc giống nhau
        setCornerRadius(radius, radius, radius, radius);
        update();
    }

    // Set kích thước
    void setSize(const sf::Vector2f& size) {
        mySize = size;
        update();
    }
    const sf::Vector2f& getSize() const { return mySize; }

    // Set màu
    void setFillColor(const sf::Color& color) {
        myShape.setFillColor(color);
    }
    const sf::Color& getFillColor() const { return myShape.getFillColor(); }

    // Set bo từng góc: TopLeft, TopRight, BottomRight, BottomLeft
    void setCornerRadius(float tl, float tr, float br, float bl) {
        radii[0] = tl;
        radii[1] = tr;
        radii[2] = br;
        radii[3] = bl;
        update();
    }

    // Lấy bounds để check click
    sf::FloatRect getGlobalBounds() const {
        return getTransform().transformRect(myShape.getGlobalBounds());
    }

private:
    sf::Vector2f mySize;
    float radii[4]; // 0:TL, 1:TR, 2:BR, 3:BL
    sf::ConvexShape myShape;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(myShape, states);
    }

    void update() {
        // Nếu size quá nhỏ, không vẽ hoặc vẽ rect thường
        if (mySize.x <= 0 || mySize.y <= 0) return;

        // Tính toán các điểm cho ConvexShape
        // Số điểm cho mỗi góc bo (càng cao càng mượt)
        const int quality = 30;
        const float PI = 3.14159265f;

        myShape.setPointCount(quality * 4);
        int idx = 0;

        // 1. Top Right (Góc phần tư 1: 270 -> 360 độ / -90 -> 0)
        // Tâm: (W - r, r)
        float r = radii[1];
        sf::Vector2f center(mySize.x - r, r);
        for (int i = 0; i < quality; ++i) {
            float angle = -PI / 2.0f + (PI / 2.0f) * ((float)i / (quality - 1));
            myShape.setPoint(idx++, center + sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
        }

        // 2. Bottom Right (0 -> 90)
        // Tâm: (W - r, H - r)
        r = radii[2];
        center = sf::Vector2f(mySize.x - r, mySize.y - r);
        for (int i = 0; i < quality; ++i) {
            float angle = 0.0f + (PI / 2.0f) * ((float)i / (quality - 1));
            myShape.setPoint(idx++, center + sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
        }

        // 3. Bottom Left (90 -> 180)
        // Tâm: (r, H - r)
        r = radii[3];
        center = sf::Vector2f(r, mySize.y - r);
        for (int i = 0; i < quality; ++i) {
            float angle = PI / 2.0f + (PI / 2.0f) * ((float)i / (quality - 1));
            myShape.setPoint(idx++, center + sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
        }

        // 4. Top Left (180 -> 270)
        // Tâm: (r, r)
        r = radii[0];
        center = sf::Vector2f(r, r);
        for (int i = 0; i < quality; ++i) {
            float angle = PI + (PI / 2.0f) * ((float)i / (quality - 1));
            myShape.setPoint(idx++, center + sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
        }
    }
};

} // namespace GUI
