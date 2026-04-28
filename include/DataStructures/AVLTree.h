#pragma once
#include "DataStructure.h"
#include "ISnapshot.h"
#include <vector>
#include <string>
#include <set>

namespace DS
{
    struct AVLNode
    {
        int data;
        int id;
        int height;
        AVLNode* left;
        AVLNode* right;

        AVLNode(int val, int nodeId)
            : data(val), id(nodeId), height(0), left(nullptr), right(nullptr) {}
    };

    class AVLTree : public DataStructure
    {
    private:
        AVLNode* m_root;
        int m_nodeCounter;

        // Cấu hình hiển thị
        const float VERTICAL_GAP = 120.0f;
        const float HORIZONTAL_SPACING = 500.0f;

        // Helpers thuật toán
        int getHeight(AVLNode* n);
        int getBalance(AVLNode* n);
        void updateHeight(AVLNode* n);
        AVLNode* findMin(AVLNode* n);

        // Phép xoay (Sử dụng ArcSwing cho mượt)
        AVLNode* rightRotate(AVLNode* y, const std::string& title);
        AVLNode* leftRotate(AVLNode* x, const std::string& title);

        // Core logic đệ quy
        AVLNode* insertRecursive(AVLNode* node, int val, const std::string& title);
        AVLNode* removeRecursive(AVLNode* node, int val, const std::string& title);

        // Visualization Helpers
        void createSnapshot(const std::string& title, const std::string& sub,
                            GUI::Scenario sce, const std::string& key, int line,
                            std::vector<std::pair<std::string, std::string>> vars,
                            int focusId = -1, std::set<int> warningIds = {});

        void generateStates(AVLNode* node, sf::Vector2f pos, float offset,
                            std::vector<Core::NodeState>& nStates,
                            std::vector<Core::EdgeState>& eStates,
                            int focusId, std::set<int> warningIds);

    public:
        AVLTree();
        ~AVLTree();

        void insert(int val);
        void remove(int val);
        void search(int val);
        void clear();

        bool loadFromFile(const std::string& path) override;
        std::vector<Command> getCommands() override;
        std::string getName() const override { return "AVL Tree"; }
    };
}
