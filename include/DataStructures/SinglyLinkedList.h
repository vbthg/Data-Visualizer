#pragma once
#include "DataStructure.h"
#include "Squircle.h"
#include "Theme.h"
#include "Spring.h" // Dùng Spring cho mượt
#include <vector>

namespace DS {

class SinglyLinkedList : public DataStructure
{
private:
    // --- Inner Class: Visual Node ---
    struct Node
    {
        int value;
        Node* next;

        // Graphics
        GUI::Squircle shape;
        sf::Text label;

        // Animation Physics (Thay vì Lerp thủ công)
        Utils::Physics::Spring xSpring;
        Utils::Physics::Spring ySpring;

        Node(int val, const sf::Font& font); // Constructor

        // Update vật lý & Graphics
        void update(float dt);

        // Set vị trí ngay lập tức (không animation)
        void snapTo(sf::Vector2f pos);

        // Set mục tiêu để lò xo tự chạy tới
        void moveTo(sf::Vector2f target);

        sf::Vector2f getPosition() const;
        sf::Vector2f getSize() const;

        void draw(sf::RenderWindow& window);
    };

    // --- Members ---
    Node* head;
    sf::Font& font; // Reference tới font từ ResourceManager

    // --- Helpers ---
    void realignNodes(); // Tính toán lại vị trí các node
    void drawArrows(sf::RenderWindow& window);

    // Logic thực tế
    void insertHead(int val);
    void insertTail(int val);
    void deleteHead();
    void clear();

public:
    SinglyLinkedList();
    ~SinglyLinkedList();

    std::string getName() const override;
    std::vector<DS::Command> getCommands() override;

    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
};

} // namespace DS
