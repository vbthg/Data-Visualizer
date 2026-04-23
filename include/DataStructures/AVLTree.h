#pragma once

#include "DataStructure.h"
#include "TimelineManager.h"
#include "Snapshot.h"
#include "Command.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace DS
{
    class AVLTree : public DataStructure
    {
    private:
        struct Node
        {
            int data;
            int height;
            int id; // ID duy nhất và cố định để phục vụ nội suy animation
            Node* left;
            Node* right;

            Node(int val, int nodeId)
                : data(val), height(1), id(nodeId), left(nullptr), right(nullptr) {}
        };

        Node* m_root;
        int m_nextId;

        // --- Cấu hình Layout (Tọa độ SFML) ---
        const float m_initialHorizontalGap = 450.0f;
        const float m_verticalGap = 130.0f;
        const float m_startY = 120.0f;

    public:
        AVLTree();
        virtual ~AVLTree();

        // --- Hệ thống Command UI ---
        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "AVL Tree"; }

        // --- Chức năng chính ---
        void insert(int value);
        void remove(int value);
        bool search(int value);
        void updateNodeValue(int oldVal, int newVal);
        void clear();

    private:
        // --- Helper Thuật toán (Đệ quy) ---
        Node*& insertRecursive(Node*& node, int value, bool& isInserted);
        Node*& removeRecursive(Node*& node, int value, bool& isRemoved);

        // --- Logic Cân bằng & Xoay ---
        void balance(Node*& n);
        void rotateLeft(Node*& x);
        void rotateRight(Node*& y);

        // --- Helper Tiện ích ---
        int getHeight(Node* n);
        int getBalanceFactor(Node* n);
        void updateHeight(Node* n);
        Node* findMin(Node* n);
        void clearRecursive(Node* node);

        // --- Hệ thống Visualizing (Snapshot Logic) ---

        /**
         * @brief Chụp ảnh trạng thái cây hiện tại.
         * @param highlightNodeId ID của node cần làm nổi bật (ví dụ node đang so sánh).
         * @param message Lời giải thích bước hiện tại sẽ hiện trên Notch.
         */
        void createSnapshot(int highlightNodeId = -1, const std::string& message = "", bool isWarning = false, int pulseChildId = -1);

        /**
         * @brief Tính toán vị trí từng node trên không gian 2D để không chồng chéo.
         * @param hGap Khoảng cách ngang giảm dần theo độ sâu (Geometric Decay).
         */
        void computeLayout(Node* node, float x, float y, int depth, int maxDepth,
                            std::vector<Core::NodeState>& nodes,
                            std::vector<Core::EdgeState>& edges);
    };
}
