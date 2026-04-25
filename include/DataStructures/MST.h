#pragma once
#include "DataStructure.h"
#include "ISnapshot.h"
#include <vector>
#include <string>
#include <map>

namespace DS
{
    class MST : public DataStructure
    {
    private:
        // Cấu trúc cạnh đồ thị
        struct Edge
        {
            int id;
            int u, v;
            float weight;
        };

        // Cấu trúc đỉnh đồ thị
        struct Node
        {
            int id;
            sf::Vector2f pos;
        };

        // Cấu trúc DSU để kiểm tra chu trình (Cycle Detection)
        struct DSU
        {
            std::vector<int> parent;
            std::vector<int> size; // Thêm biến theo dõi kích thước tập hợp
            DSU(int n)
            {
                parent.resize(n);
                size.assign(n, 1); // Ban đầu mỗi node là 1 tập hợp size 1
                for(int i = 0; i < n; ++i) parent[i] = i;
            }
            int find(int i)
            {
                if(parent[i] == i) return i;
                return parent[i] = find(parent[i]);
            }
            // Sửa lại hàm unite để trả về thông tin đỉnh nào là "nguồn"
            bool unite(int i, int j)
            {
                int root_i = find(i);
                int root_j = find(j);
                if(root_i != root_j)
                {
                    // Hợp nhất tập nhỏ vào tập lớn
                    if(size[root_i] < size[root_j]) std::swap(root_i, root_j);
                    parent[root_j] = root_i;
                    size[root_i] += size[root_j];
                    return true;
                }
                return false;
            }
        };

        bool isPositionSafe(sf::Vector2f pos, float minDistance);
        sf::Vector2f findBestPosition(sf::Vector2f pivotPos);

        std::vector<Node> m_nodes;
        std::vector<Edge> m_edges;
        int m_nextNodeId = 0;
        int m_nextEdgeId = 0;

    public:
        MST();
        virtual ~MST() = default;

        // --- COMMAND SYSTEM ---
        std::vector<Command> getCommands() override;
        std::string getName() const override { return "Kruskal's MST"; }

        // --- LOGIC OPERATIONS ---
        void addNode(sf::Vector2f pos);
        void addEdge(int u, int v, float weight);
        void runKruskal();
        void clear();

        void updateNodePosition(int id, sf::Vector2f newPos);

    private:
        /**
         * @brief Chụp trạng thái hiện tại, đóng gói cho Notch và CodeBox.
         * * @param scenario Kịch bản cho Notch (Processing, Success, Warning...)
         * @param message Thông báo hiện trên Notch và Log.
         * @param lineIdx Dòng code highlight trong PseudoCodeBox.
         * @param mstEdgeIds Danh sách ID các cạnh đã thuộc MST.
         * @param currentEdgeId ID cạnh đang xét (để Pulse).
         * @param rejectedEdgeId ID cạnh vừa bị loại (để đổi màu Đỏ).
         */
        void createSnapshot(GUI::Scenario scenario,
                           const std::string& message,
                           int lineIdx,
                           const std::vector<int>& mstEdgeIds,
                           int currentEdgeId = -1,
                           int sourceNodeId = -1, // Thêm: Đỉnh bắt đầu đổ màu
                           int rejectedEdgeId = -1,
                           std::vector<std::pair<std::string, std::string>> vars = {},
                           bool resetProgress = false);
    };
}
