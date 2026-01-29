namespace Utils { namespace Graphics {

    inline void drawRoundedRect(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, float radius, sf::Color color)
    {
        // 1. VẼ 4 GÓC TRÒN (Corners)
        sf::CircleShape corner(radius);
        corner.setFillColor(color);

        // Góc trên-trái (Top-Left)
        corner.setPosition(pos.x, pos.y);
        window.draw(corner);

        // Góc trên-phải (Top-Right)
        corner.setPosition(pos.x + size.x - 2 * radius, pos.y);
        window.draw(corner);

        // Góc dưới-phải (Bottom-Right)
        corner.setPosition(pos.x + size.x - 2 * radius, pos.y + size.y - 2 * radius);
        window.draw(corner);

        // Góc dưới-trái (Bottom-Left)
        corner.setPosition(pos.x, pos.y + size.y - 2 * radius);
        window.draw(corner);

        // 2. VẼ THÂN GIỮA (Center Body)
        // Chiếm toàn bộ chiều cao, nhưng chiều rộng trừ đi phần bo góc 2 bên
        // Vị trí: Lùi vào trong 1 khoảng radius
        sf::RectangleShape centerRect(sf::Vector2f(size.x - 2 * radius, size.y));
        centerRect.setPosition(pos.x + radius, pos.y);
        centerRect.setFillColor(color);
        window.draw(centerRect);

        // 3. VẼ CẠNH TRÁI (Left Edge)
        // Nằm kẹp giữa góc trên-trái và dưới-trái
        // Rộng: radius, Cao: size.y - 2*radius
        sf::RectangleShape leftRect(sf::Vector2f(radius, size.y - 2 * radius));
        leftRect.setPosition(pos.x, pos.y + radius);
        leftRect.setFillColor(color);
        window.draw(leftRect);

        // 4. VẼ CẠNH PHẢI (Right Edge)
        // Nằm kẹp giữa góc trên-phải và dưới-phải
        sf::RectangleShape rightRect(sf::Vector2f(radius, size.y - 2 * radius));
        rightRect.setPosition(pos.x + size.x - radius, pos.y + radius);
        rightRect.setFillColor(color);
        window.draw(rightRect);
    }
}}
