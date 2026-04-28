#pragma once
#include "DataStructure.h"
#include <vector>
#include <string>

namespace DS
{
    struct Node
    {
        int id;
        int value;
        sf::Vector2f pos;
    };

    class SinglyLinkedList : public DataStructure
    {
    private:
        std::vector<Node> m_nodes;
        int m_headId;
        int m_nextNodeId;

        const float NODE_DISTANCE = 200.0f;
        const float Y_LEVEL = 540.0f;
        const int GHOST_NULL_ID = -999;

    public:
        SinglyLinkedList();
        virtual ~SinglyLinkedList();

        void pushFront(int value);
        void pushBack(int value);
        void insert(int index, int value);
        void remove(int index);

        void createSnapshot(const std::string& title,
                            GUI::Scenario scenario,
                            const std::string& message,
                            const std::string& macroKey,
                            int lineIdx,
                            int highlightNodeId = -1,
                            std::vector<std::pair<std::string, std::string>> vars = {},
                            float customNullX = -1);

        void recomputePositions();

        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "SINGLY LINKED LIST"; }
        virtual void updateNodePosition(int id, sf::Vector2f newPos) override;
        bool loadFromFile(const std::string& path);
    };
}
