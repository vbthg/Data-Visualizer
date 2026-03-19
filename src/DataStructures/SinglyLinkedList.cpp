#include "SinglyLinkedList.h"
#include "ResourceManager.h" // Singleton Resource
#include <iostream>

namespace DS {

// --- Node Implementation ---

SinglyLinkedList::Node::Node(int val, const sf::Font& fontRes)
    : value(val), next(nullptr)
{
    // 1. Setup Visual
    shape.setSize({80.0f, 50.0f});
    shape.setRadius(15.0f);
    shape.setFillColor(sf::Color::White); // Trắng sứ

    // Viền mỏng (Hairline) dùng hàm bạn mới thêm
    shape.setOutlineThickness(-1.0f);
    shape.setOutlineColor(sf::Color(200, 200, 200));

    // 2. Setup Text (Oversampling trick)
    label.setFont(fontRes);
    label.setString(std::to_string(val));
    label.setCharacterSize(40); // Render to
    label.setScale(0.5f, 0.5f); // Scale nhỏ
    label.setFillColor(Utils::Graphics::Theme::Color::TextPrimary); // Đen đậm

    // Căn giữa text sơ bộ (sẽ chỉnh lại trong update)
    sf::FloatRect b = label.getLocalBounds();
    label.setOrigin(b.left + b.width/2.0f, b.top + b.height/2.0f);

    // 3. Setup Physics (Lò xo mềm hơn Button một chút để di chuyển mượt)
    xSpring.stiffness = 200.0f;
    xSpring.damping = 20.0f;

    ySpring.stiffness = 200.0f;
    ySpring.damping = 20.0f;
}

void SinglyLinkedList::Node::snapTo(sf::Vector2f pos)
{
    xSpring.snapTo(pos.x);
    ySpring.snapTo(pos.y);
    shape.setPosition(pos);
}

void SinglyLinkedList::Node::moveTo(sf::Vector2f target)
{
    xSpring.target = target.x;
    ySpring.target = target.y;
}

void SinglyLinkedList::Node::update(float dt)
{
    // Update lò xo
    xSpring.update(dt);
    ySpring.update(dt);

    float curX = xSpring.position;
    float curY = ySpring.position;

    // Apply vào Graphics
    shape.setPosition(curX, curY);

    // Text luôn ở tâm Shape
    // getSize() trả về Vector2f
    sf::Vector2f size = shape.getSize();
    label.setPosition(curX + size.x / 2.0f, curY + size.y / 2.0f - 2.0f); // -2.0f để căn chỉnh mắt nhìn
}

sf::Vector2f SinglyLinkedList::Node::getPosition() const {
    return shape.getPosition();
}

sf::Vector2f SinglyLinkedList::Node::getSize() const {
    return shape.getSize();
}

void SinglyLinkedList::Node::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(label);
}

// --- SinglyLinkedList Implementation ---

SinglyLinkedList::SinglyLinkedList()
    : head(nullptr),
      // Lấy font từ ResourceManager
      font(ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf"))
{
}

SinglyLinkedList::~SinglyLinkedList()
{
    clear();
}

std::string SinglyLinkedList::getName() const
{
    return "Singly Linked List";
}

std::vector<DS::Command> SinglyLinkedList::getCommands()
{
    std::vector<DS::Command> cmds;

    // Add Head
    cmds.push_back(DS::Command("Add Head", DS::InputType::Integer, [this](DS::InputArgs args){
        this->insertHead(args.iVal1);
    }));

    // Add Tail
    cmds.push_back(DS::Command("Add Tail", DS::InputType::Integer, [this](DS::InputArgs args){
        this->insertTail(args.iVal1);
    }));

    // Del Head
    cmds.push_back(DS::Command("Del Head", DS::InputType::None, [this](DS::InputArgs){
        this->deleteHead();
    }));

    // Clear
    cmds.push_back(DS::Command("Clear", DS::InputType::None, [this](DS::InputArgs){
        this->clear();
    }));

    return cmds;
}

void SinglyLinkedList::update(float dt)
{
    Node* curr = head;
    while(curr)
    {
        curr->update(dt);
        curr = curr->next;
    }
}

void SinglyLinkedList::draw(sf::RenderTarget& target)
{
    // Vẽ Layer 1: Mũi tên (Dưới cùng)
    drawArrows(target);

    // Vẽ Layer 2: Node (Trên cùng)
    Node* curr = head;
    while(curr)
    {
        curr->draw(target);
        curr = curr->next;
    }
}

// --- LOGIC ---

void SinglyLinkedList::insertHead(int val)
{
    if (onLog) onLog("Khoi tao Node moi voi gia tri: " + std::to_string(val));

    Node* newNode = new Node(val, font);
    newNode->next = head;
    head = newNode;

    // Animation: Bay từ trên cao xuống
    // Giả định màn hình rộng ~1200, ta cho nó xuất hiện ở giữa
    newNode->snapTo({100.0f, -100.0f});

    realignNodes();

    if (onLog) onLog("Da chen thanh cong!");
}

void SinglyLinkedList::insertTail(int val)
{
    Node* newNode = new Node(val, font);

    if(!head)
    {
        head = newNode;
        newNode->snapTo({100.0f, -100.0f});
    }
    else
    {
        Node* temp = head;
        while(temp->next) temp = temp->next;
        temp->next = newNode;

        // Node mới xuất hiện ngay tại vị trí node cuối (hiệu ứng nảy ra)
        newNode->snapTo(temp->getPosition());
    }

    realignNodes();
}

void SinglyLinkedList::deleteHead()
{
    if(!head) return;
    Node* temp = head;
    head = head->next;
    delete temp;

    realignNodes();
}

void SinglyLinkedList::clear()
{
    while(head) deleteHead();
}

void SinglyLinkedList::realignNodes()
{
    Node* curr = head;
    float startX = 150.0f; // Padding trái
    float startY = 300.0f; // Giữa màn hình (theo chiều dọc)
    float gap = 140.0f;    // Khoảng cách rộng ra cho mũi tên

    int index = 0;
    while(curr)
    {
        // Chỉ set target, lò xo sẽ tự lo phần di chuyển
        curr->moveTo({startX + index * gap, startY});

        curr = curr->next;
        index++;
    }
}

void SinglyLinkedList::drawArrows(sf::RenderWindow& window)
{
    Node* curr = head;
    while(curr && curr->next)
    {
        // 1. Lấy tọa độ mép phải node hiện tại
        sf::Vector2f start = curr->getPosition();
        start.x += curr->getSize().x; // Mép phải
        start.y += curr->getSize().y / 2.0f; // Giữa theo chiều dọc

        // 2. Lấy tọa độ mép trái node tiếp theo
        sf::Vector2f end = curr->next->getPosition();
        end.y += curr->next->getSize().y / 2.0f;

        // 3. Vẽ BEZIER CURVE (Đơn giản hóa bằng VertexArray)
        // Logic: 2 điểm điều khiển tạo hình chữ S mềm mại
        sf::Vector2f cp1 = start + sf::Vector2f(30.0f, 0.0f); // Kéo sang phải
        sf::Vector2f cp2 = end - sf::Vector2f(30.0f, 0.0f);   // Kéo sang trái

        sf::VertexArray curve(sf::LineStrip);

        // Chia đường cong thành 20 đoạn
        int segments = 20;
        for(int i = 0; i <= segments; ++i)
        {
            float t = (float)i / segments;

            // Công thức Bezier bậc 3
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            sf::Vector2f p = uuu * start + 3 * uu * t * cp1 + 3 * u * tt * cp2 + ttt * end;

            // Màu mũi tên: Xám nhạt
            curve.append(sf::Vertex(p, sf::Color(180, 180, 180)));
        }

        window.draw(curve);

        curr = curr->next;
    }
}

} // namespace DS
