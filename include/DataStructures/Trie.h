#pragma once
#include "DataStructure.h"
#include "GUI/NotchEnums.h"
#include <map>
#include <string>
#include <vector>

namespace DS
{
    class Trie : public DataStructure
    {
    private:
        struct Node
        {
            int id;
            char character;
            bool isEndOfWord;
            std::map<char, Node*> children;

            Node(char c, int nodeId) : character(c), id(nodeId), isEndOfWord(false) {}
        };

        Node* m_root;
        int m_nextId;
        float m_startY = 100.0f;

        // Palette màu Apple Design tương tự ShortestPath
        const sf::Color COLOR_DEFAULT = sf::Color(255, 255, 255);
        const sf::Color COLOR_FOCUS   = sf::Color(255, 212, 59);  // Vàng (Đang xét)
        const sf::Color COLOR_SUCCESS = sf::Color(52, 199, 89);   // Xanh lá (EndOfWord)
        const sf::Color COLOR_ERROR   = sf::Color(255, 69, 58);   // Đỏ (Lỗi/Không thấy)
        const sf::Color COLOR_TEXT    = sf::Color::Black;

    public:
        Trie();
        virtual ~Trie();

        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "TRIE (PREFIX TREE)"; }

        void insert(const std::string& word);
        bool search(const std::string& word);
        void remove(const std::string& word);
        void clear();

        // Định dạng file: Mỗi dòng là một từ (string)
        virtual bool loadFromFile(const std::string& path) override;

    private:
        bool removeRecursive(Node* curr, const std::string& word, int index, bool& deleted);
        void clearRecursive(Node* node);

        // Snapshot System cập nhật theo Syntax ISnapshot mới
        void createSnapshot(
            GUI::Scenario scenario,
            const std::string& title,
            const std::string& subtitle,
            const std::string& macroKey,
            int lineIdx,
            int highlightNodeId = -1,
            std::vector<std::pair<std::string, std::string>> vars = {}
        );

        void computeLayout(Node* node, float x, float y, float horizontalRange,
                           std::vector<Core::NodeState>& nodes,
                           std::vector<Core::EdgeState>& edges);

        Node* findNode(int id, Node* startNode); // Helper để tìm node trong tree

        // Tính toán độ rộng cần thiết cho mỗi node (đệ quy)
        float calculateSubtreeWidth(Node* node, std::map<int, float>& widthMap);

        // Hằng số định nghĩa khoảng cách
        const float NODE_GAP = 70.0f;
    };
}
