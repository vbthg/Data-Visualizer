#include "SinglyLinkedList.h"
#include <iostream>
#include <sstream>

namespace DS
{
    SinglyLinkedList::SinglyLinkedList()
    {
        m_headId = -1;
        m_nextNodeId = 0;

        // Khởi tạo danh sách trống với duy nhất một Node NULL
        createSnapshot(GUI::Scenario::Initial, "Khởi tạo danh sách liên kết trống", 0);
    }

    SinglyLinkedList::~SinglyLinkedList()
    {
    }

    void SinglyLinkedList::recomputePositions()
    {
        // Tính toán lại vị trí X cho từng Node dựa trên thứ tự trong vector
        // Node thứ i sẽ có x = i * 200
        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            m_nodes[i].pos.x = i * NODE_DISTANCE;
            m_nodes[i].pos.y = Y_LEVEL;
        }
    }

void SinglyLinkedList::pushFront(int value)
    {
        m_timeline->onNewMacroStarted();

        // 1. Prepare logic
        int newId = m_nextNodeId++;
        Node newNode = {newId, value, {0.0f, Y_LEVEL}}; // Target position is (0, Y)

        // 2. Insert and Recompute IMMEDIATELY
        m_nodes.insert(m_nodes.begin(), newNode);
        m_headId = newId;
        recomputePositions();

        // 3. To create the "Gap" effect, we manually lift the new node up for the first snapshot
        // but since recomputePositions was called, all other nodes already have their new X targets.
        m_nodes[0].pos.y -= 150.0f;

        // Snapshot 1: Old nodes slide right, New node appears high at x=0
        createSnapshot(GUI::Scenario::Processing,
                      "Creating new node and shifting list to create space", 0, newId,
                      {{"newNode", "Node(" + std::to_string(value) + ")"}, {"head", "Node(" + std::to_string(m_nodes[1].value) + ")"}});

        // Snapshot 2: Node settles down and connections stabilize
        m_nodes[0].pos.y += 150.0f;
        createSnapshot(GUI::Scenario::Success, "Updating head pointer and finalizing insertion", 3, newId);
    }

    void SinglyLinkedList::pushBack(int value)
    {
        m_timeline->onNewMacroStarted();

        int newId = m_nextNodeId++;
        // Tạo Node mới ở cuối (trước Node NULL)
        Node newNode = {newId, value, {static_cast<float>(m_nodes.size()) * NODE_DISTANCE, Y_LEVEL - 150.0f}};

        m_nodes.push_back(newNode);
        if(m_nodes.size() == 1) m_headId = newId;

        createSnapshot(GUI::Scenario::Processing, "Thêm Node vào cuối danh sách", 1, newId);

        recomputePositions();
        createSnapshot(GUI::Scenario::Success, "Nối dây vào Node mới", 2);
    }

void SinglyLinkedList::insert(int index, int value)
    {
        if(index < 0 || index > (int)m_nodes.size()) return;
        if(index == 0)
        {
            pushFront(value);
            return;
        }

        m_timeline->onNewMacroStarted();

        // 1. Traverse with yellow highlight
        for(int i = 0; i < index; ++i)
        {
            createSnapshot(GUI::Scenario::Processing, "Traversing to index " + std::to_string(index), 2, m_nodes[i].id,
                          {{"i", std::to_string(i)}, {"curr", "Node(" + std::to_string(m_nodes[i].value) + ")"}});
        }

        // 2. Insert and create gap immediately
        int newId = m_nextNodeId++;
        Node newNode = {newId, value, {index * NODE_DISTANCE, Y_LEVEL}};
        m_nodes.insert(m_nodes.begin() + index, newNode);
        recomputePositions();

        // Lift for animation
        m_nodes[index].pos.y -= 150.0f;

        createSnapshot(GUI::Scenario::Processing, "Creating space and linking new node", 7, newId,
                      {{"newNode", "Node(" + std::to_string(value) + ")"}, {"prev", "Node(" + std::to_string(m_nodes[index-1].value) + ")"}});

        // 3. Settle
        m_nodes[index].pos.y += 150.0f;
        createSnapshot(GUI::Scenario::Success, "Insertion completed", 8, newId);
    }

    void SinglyLinkedList::remove(int index)
    {
        if(index < 0 || index >= (int)m_nodes.size()) return;
        m_timeline->onNewMacroStarted();

        // 1. Traverse
        for(int i = 0; i <= index; ++i)
        {
            GUI::Scenario sc = (i == index) ? GUI::Scenario::Warning : GUI::Scenario::Processing;
            std::string msg = (i == index) ? "Target node identified" : "Searching for node...";

            createSnapshot(sc, msg, 2, m_nodes[i].id,
                          {{"i", std::to_string(i)}, {"target", std::to_string(index)}});
        }

        // 2. Remove logic
        m_nodes.erase(m_nodes.begin() + index);
        if(index == 0) m_headId = m_nodes.empty() ? -1 : m_nodes[0].id;

        // 3. Shift back immediately
        recomputePositions();
        createSnapshot(GUI::Scenario::Success, "Bypassing node and shifting list to fill the gap", 9);
    }

    // Hàm createSnapshot nâng cấp để nhận thêm biến cục bộ
    void SinglyLinkedList::createSnapshot(GUI::Scenario scenario,
                                         const std::string& message,
                                         int lineIdx,
                                         int highlightNodeId,
                                         std::vector<std::pair<std::string, std::string>> vars,
                                         float customNullX)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        snap->scenario = scenario;
        snap->logMessage = message;
        snap->operationName = "SINGLY LINKED LIST";
        snap->macroKey = "singly_linked_list";
        snap->pseudoCodeLine = lineIdx;
        snap->variableStates = vars; // Đưa biến cục bộ vào đây để CodeBox hiển thị

        // 1. Snapshot các Node dữ liệu
        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            Core::NodeState ns;
            ns.id = m_nodes[i].id;
            ns.position = m_nodes[i].pos;
            ns.value = std::to_string(m_nodes[i].value);
            ns.isDraggable = false; // Khóa kéo thả theo yêu cầu của em

            // Hiển thị nhãn HEAD ở Node đầu tiên
            if(ns.id == m_headId)
            {
                ns.subText = "HEAD";
            }

            // Màu sắc highlight khi đang duyệt
            if(ns.id == highlightNodeId)
            {
                ns.fillColor = sf::Color(255, 212, 59); // Màu Vàng Focus
                ns.scale = 1.2f;
            }

            snap->nodeStates.push_back(ns);
        }

        // 2. Thêm Ghost Node NULL ở cuối đoàn tàu
        Core::NodeState nullSnap;
        nullSnap.id = GHOST_NULL_ID;
        nullSnap.value = "NULL";
        // Nếu customNullX < 0, mặc định tính theo vị trí Node cuối cùng hiện tại
        if(customNullX < 0)
        {
            nullSnap.position = {static_cast<float>(m_nodes.size()) * NODE_DISTANCE, Y_LEVEL};
        }
        else
        {
            nullSnap.position = {customNullX, Y_LEVEL};
        }
        nullSnap.fillColor = sf::Color(60, 60, 60);
        nullSnap.textColor = sf::Color::White;
        nullSnap.opacity = 0.7f;
        nullSnap.isDraggable = false;
        snap->nodeStates.push_back(nullSnap);

        // 3. Snapshot các cạnh (Edges)
        // Nối từ Node i sang Node i+1
        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            Core::EdgeState es;
            es.startNodeId = m_nodes[i].id;
            // Nếu là Node cuối thì nối tới Ghost NULL, ngược lại nối tới Node sau
            es.endNodeId = (i == m_nodes.size() - 1) ? GHOST_NULL_ID : m_nodes[i+1].id;

            es.thickness = 4.0f;
            es.fillProgress = 1.0f; // Luôn hiện dây
            snap->edgeStates.push_back(es);
        }

        if(m_timeline) m_timeline->addSnapshot(snap);
    }

    std::vector<Command> SinglyLinkedList::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command("Push Front", InputType::Integer, [this](InputArgs args) {
            this->pushFront(args.iVal1);
        }));
        cmds.push_back(Command("Push Back", InputType::Integer, [this](InputArgs args) {
            this->pushBack(args.iVal1);
        }));
        cmds.push_back(Command("Insert (idx, val)", InputType::TwoIntegers, [this](InputArgs args) {
            this->insert(args.iVal1, args.iVal2);
        }));
        cmds.push_back(Command("Remove (idx)", InputType::Integer, [this](InputArgs args) {
            this->remove(args.iVal1);
        }));
        cmds.push_back(Command("Clear", InputType::None, [this](InputArgs args) {
            this->m_nodes.clear();
            this->m_headId = -1;
            this->m_timeline->onNewMacroStarted();
            this->createSnapshot(GUI::Scenario::Success, "Đã xóa toàn bộ danh sách", 0);
        }));
        return cmds;
    }

    void SinglyLinkedList::updateNodePosition(int id, sf::Vector2f newPos)
    {
        // Vì isDraggable = false nên hàm này có thể để trống hoặc chỉ dùng cho nội bộ
    }

} // namespace DS
