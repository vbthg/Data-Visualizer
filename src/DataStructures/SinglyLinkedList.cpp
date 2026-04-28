#include "SinglyLinkedList.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

namespace DS
{
    SinglyLinkedList::SinglyLinkedList()
    {
        m_headId = -1;
        m_nextNodeId = 0;
    }

    SinglyLinkedList::~SinglyLinkedList()
    {}

    void SinglyLinkedList::recomputePositions()
    {
        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            m_nodes[i].pos.x = 400.0f + i * NODE_DISTANCE;
            m_nodes[i].pos.y = Y_LEVEL;
        }
    }

    void SinglyLinkedList::pushFront(int value)
    {
        std::string opTitle = "Push Front " + std::to_string(value);
        m_timeline->onNewMacroStarted();

        // Snapshot Start: Preparing
        createSnapshot(opTitle, GUI::Scenario::Idle, "Preparing to push front " + std::to_string(value), "sll_push_front", 0);

        int newId = m_nextNodeId++;
        Node newNode = {newId, value, {400.0f, Y_LEVEL - 150.0f}};

        m_nodes.insert(m_nodes.begin(), newNode);
        m_headId = newId;
        recomputePositions();
        m_nodes.front().pos.y = Y_LEVEL - 150.f;

        // Snapshot Processing
        createSnapshot(opTitle, GUI::Scenario::Processing,
                       "Create new node: newNode = new Node(" + std::to_string(value) + ")", "sll_push_front", 0, newId,
                       {{"newNode", "Node(" + std::to_string(value) + ")" }});

        // Snapshot End: Completed
        recomputePositions();
        createSnapshot(opTitle, GUI::Scenario::Success, "Completed push front", "sll_push_front", 2);

        m_timeline->onMacroFinished();
    }

    void SinglyLinkedList::pushBack(int value)
    {
        std::string opTitle = "Push Back " + std::to_string(value);
        m_timeline->onNewMacroStarted();

        // Snapshot Start: Preparing
        createSnapshot(opTitle, GUI::Scenario::Idle, "Preparing to push back " + std::to_string(value), "sll_insert", 0);

        int newId = m_nextNodeId++;
        Node newNode = {newId, value, {400.0f + static_cast<float>(m_nodes.size()) * NODE_DISTANCE, Y_LEVEL - 150.0f}};
        m_nodes.push_back(newNode);
        if(m_nodes.size() == 1)
        {
            m_headId = newId;
        }

        createSnapshot(opTitle, GUI::Scenario::Processing, "Place new node at the end", "sll_insert", 1, newId);

        // Snapshot End: Completed
        recomputePositions();
        createSnapshot(opTitle, GUI::Scenario::Success, "Completed push back", "sll_insert", 8);

        m_timeline->onMacroFinished();
    }

    void SinglyLinkedList::insert(int index, int value)
    {
        if(index < 0 || index > (int)m_nodes.size())
        {
            return;
        }
        if(index == 0)
        {
            pushFront(value);
            return;
        }

        std::string opTitle = "Insert " + std::to_string(value) + " at " + std::to_string(index);
        m_timeline->onNewMacroStarted();

        // Snapshot Start: Preparing
        createSnapshot(opTitle, GUI::Scenario::Idle, "Preparing to insert at index " + std::to_string(index), "sll_insert", 0);

        for(int i = 0; i < index; ++i)
        {
            createSnapshot(opTitle, GUI::Scenario::Processing, "Traversing list... current index: " + std::to_string(i), "sll_insert", 3, m_nodes[i].id,
                          {{"i", std::to_string(i)}, {"curr", std::to_string(m_nodes[i].value)}});
        }

        int newId = m_nextNodeId++;
        Node newNode = {newId, value, {400.0f + index * NODE_DISTANCE, Y_LEVEL - 150.0f}};
        m_nodes.insert(m_nodes.begin() + index, newNode);

        for(size_t j = 0; j < m_nodes.size(); ++j)
        {
            m_nodes[j].pos.x = 400.0f + j * NODE_DISTANCE;
        }
        m_nodes[index].pos.y = Y_LEVEL - 150.0f;

        createSnapshot(opTitle, GUI::Scenario::Processing, "Create space and prepare new node", "sll_insert", 7, newId);

        // Snapshot End: Completed
        recomputePositions();
        createSnapshot(opTitle, GUI::Scenario::Success, "Completed insert", "sll_insert", 8);

        m_timeline->onMacroFinished();
    }

    void SinglyLinkedList::remove(int index)
    {
        if(index < 0 || index >= (int)m_nodes.size())
        {
            return;
        }

        std::string opTitle = "Remove Index " + std::to_string(index);
        m_timeline->onNewMacroStarted();

        // Snapshot Start: Preparing
        createSnapshot(opTitle, GUI::Scenario::Idle, "Preparing to remove node at index " + std::to_string(index), "sll_remove", 0);

        for(int i = 0; i <= index; ++i)
        {
            createSnapshot(opTitle, GUI::Scenario::Processing, "Searching for target node...", "sll_remove", 2, m_nodes[i].id);
        }

        int deletedId = m_nodes[index].id;
        createSnapshot(opTitle, GUI::Scenario::Warning, "Node identified for deletion", "sll_remove", 7, deletedId);

        m_nodes.erase(m_nodes.begin() + index);
        if(index == 0)
        {
            m_headId = m_nodes.empty() ? -1 : m_nodes[0].id;
        }

        // Snapshot End: Completed
        recomputePositions();
        createSnapshot(opTitle, GUI::Scenario::Success, "Completed removal", "sll_remove", 10);

        m_timeline->onMacroFinished();
    }

    void SinglyLinkedList::createSnapshot(const std::string& title,
                                          GUI::Scenario scenario,
                                          const std::string& message,
                                          const std::string& macroKey,
                                          int lineIdx,
                                          int highlightNodeId,
                                          std::vector<std::pair<std::string, std::string>> vars,
                                          float customNullX)
    {
        if(!m_timeline)
        {
            return;
        }

        auto snap = std::make_shared<Core::ISnapshot>();

        snap->notchData.scenario = scenario;
        snap->notchData.title = title; // Sử dụng title cụ thể thay vì getName()
        snap->notchData.subtitle = message;
        snap->notchData.iconCode = (scenario == GUI::Scenario::Success) ? "\xef\x80\x8c" : "\xef\x84\x9e";

        snap->codeData.macroKey = macroKey;
        snap->codeData.pseudoCodeLine = lineIdx;
        snap->codeData.variableStates = vars;

        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            Core::NodeState ns;
            ns.id = m_nodes[i].id;
            ns.position = m_nodes[i].pos;
            ns.value = std::to_string(m_nodes[i].value);
            ns.isDraggable = false;

            if(ns.id == m_headId)
            {
                ns.subText = "HEAD";
            }

            if(scenario != GUI::Scenario::Success && ns.id == highlightNodeId)
            {
                ns.fillColor = (scenario == GUI::Scenario::Warning) ? sf::Color(255, 100, 100) : sf::Color(255, 212, 59);
                ns.scale = 1.2f;
            }
            snap->nodeStates.push_back(ns);
        }

        Core::NodeState nullSnap;
        nullSnap.id = GHOST_NULL_ID;
        nullSnap.value = "NULL";
        nullSnap.position = { (customNullX < 0) ? (400.0f + m_nodes.size() * NODE_DISTANCE) : customNullX, Y_LEVEL };
        nullSnap.fillColor = sf::Color(60, 60, 60);
        nullSnap.textColor = sf::Color::White;
        nullSnap.opacity = 0.7f;
        nullSnap.isDraggable = false;
        snap->nodeStates.push_back(nullSnap);

        for(size_t i = 0; i < m_nodes.size(); ++i)
        {
            Core::EdgeState es;
            es.startNodeId = m_nodes[i].id;
            es.endNodeId = (i == m_nodes.size() - 1) ? GHOST_NULL_ID : m_nodes[i+1].id;
            es.thickness = 4.0f;
            es.fillProgress = 1.0f;
            snap->edgeStates.push_back(es);
        }

        m_timeline->addSnapshot(snap);
    }

    bool SinglyLinkedList::loadFromFile(const std::string& path)
    {
        std::ifstream file(path);
        if(!file.is_open()) return false;

        // Bước 1: Thông báo cho Notch là bắt đầu nạp
        m_timeline->onNewMacroStarted();
        createSnapshot("Importing Data", GUI::Scenario::Processing, "Reading file: " + path, "sll_load", 0);

        // Bước 2: Xóa dữ liệu cũ
        m_nodes.clear();
        m_headId = -1;

        // Bước 3: Đọc dữ liệu
        int value;
        std::vector<int> tempValues;
        while(file >> value)
        {
            tempValues.push_back(value);
        }

        if(tempValues.empty()) return false;

        // Bước 4: Tái cấu trúc DS và tạo hiệu ứng nạp từng node (nếu muốn)
        for(size_t i = 0; i < tempValues.size(); ++i)
        {
            int newId = m_nextNodeId++;
            Node newNode = {newId, tempValues[i], {400.0f + i * NODE_DISTANCE, Y_LEVEL}};
            m_nodes.push_back(newNode);
            if(i == 0) m_headId = newId;

            // Cập nhật progress bar trên Notch (ví dụ: 1/10, 2/10...)
            // Ở đây em có thể gọi m_timeline->updateStep(...) nếu đã cài đặt
        }

        recomputePositions();

        // Bước 5: Hoàn tất
        createSnapshot("Import Success", GUI::Scenario::Success, "Loaded " + std::to_string(tempValues.size()) + " nodes", "sll_load", 1);
        m_timeline->onMacroFinished();

        return true;
    }

    std::vector<Command> SinglyLinkedList::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command(L"\uE024", "Push Front", InputType::Integer, [this](InputArgs args) { this->pushFront(args.iVal1); }));
        cmds.push_back(Command(L"\uE026", "Push Back", InputType::Integer, [this](InputArgs args) { this->pushBack(args.iVal1); }));
        cmds.push_back(Command(L"\uE3D6", "Insert", InputType::TwoIntegers, [this](InputArgs args) { this->insert(args.iVal1, args.iVal2); }));
        cmds.push_back(Command(L"\uE32C", "Remove", InputType::Integer, [this](InputArgs args) { this->remove(args.iVal1); }));
        cmds.push_back(Command(L"\uEC54", "Clear", InputType::None, [this](InputArgs args) {
            this->m_nodes.clear();
            this->m_headId = -1;
            this->m_timeline->onNewMacroStarted();
            this->createSnapshot("Clear List", GUI::Scenario::Success, "Completed clear", "sll_push_front", 0);
            this->m_timeline->onMacroFinished();
        }));
        return cmds;
    }

    void SinglyLinkedList::updateNodePosition(int id, sf::Vector2f newPos)
    {}
}
