#pragma once
#include "DataStructure.h"
#include "NotchEnums.h"
#include <vector>
#include <string>
#include <queue>
#include <limits>

namespace DS
{
    // Cấu trúc nội bộ để lưu trữ dữ liệu đồ thị logic
    struct GraphNode
    {
        int id;
        sf::Vector2f pos;
        float dist;     // Khoảng cách ngắn nhất hiện tại
        int parentId;   // Lưu đỉnh cha để truy vết đường đi
        bool finalized; // Đã tìm được đường ngắn nhất thực sự chưa
    };

    struct GraphEdge
    {
        int id;
        int u, v;
        float weight;
        float visualFill = 0.0f;     // Lưu lại tiến trình fill cũ
        sf::Color visualColor = sf::Color(150, 150, 150, 100); // Lưu màu cũ
        bool inPath = false;

        // Constructor giúp khởi tạo nhanh và tránh lỗi matching
        GraphEdge(int _id, int _u, int _v, float _w)
            : id(_id), u(_u), v(_v), weight(_w),
              visualFill(0.0f),
              visualColor(sf::Color(150, 150, 150, 100)),
              inPath(false) {}
    };

    class ShortestPath : public DataStructure
    {
    private:
        std::vector<GraphNode> m_nodes;
        std::vector<GraphEdge> m_edges;
        int m_nextNodeId = 0;
        int m_nextEdgeId = 0;

        // Các hằng số màu sắc chuẩn Apple/Material
        const sf::Color COLOR_DEFAULT = sf::Color::White;
        const sf::Color COLOR_FOCUS   = sf::Color(255, 212, 59);  // Vàng (Đang xét)
        const sf::Color COLOR_SUCCESS = sf::Color(52, 199, 89);   // Xanh lá (Đã chốt)
        const sf::Color COLOR_RELAX   = sf::Color(0, 122, 255);   // Xanh dương (Thư giãn cạnh)
        const sf::Color COLOR_ERROR   = sf::Color(255, 69, 58);   // Đỏ (Không thỏa mãn)

    public:
        ShortestPath();

        // --- CÁC THAO TÁC CƠ BẢN ---
        void addNode(int id, sf::Vector2f pos);
        void addEdge(int u, int v, float weight);
        void removeEdge(int u, int v);
        void clear();

        // --- THUẬT TOÁN CHÍNH ---
        void runDijkstra(int startNodeId, int endNodeId = -1);

        // --- HÀM TRỢ GIÚP VISUALIZER ---

        // Chụp lại toàn bộ trạng thái đồ thị tại 1 thời điểm
        void createSnapshot(
            GUI::Scenario scenario,
            const std::string& message,
            int lineIdx,
            int currentU = -1,
            int currentV = -1,
            int relaxingEdgeId = -1,
            std::vector<std::pair<std::string, std::string>> vars = {}
        );

        // Tìm vị trí trống để đặt node mới (tương tự MST em đã làm)
        sf::Vector2f findBestPosition(sf::Vector2f pivotPos);
        bool isPositionSafe(sf::Vector2f pos, float minDistance);

        // Override từ lớp cha DataStructure
        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "DIJKSTRA SHORTEST PATH"; }
        virtual void updateNodePosition(int id, sf::Vector2f newPos) override;

    private:
        // Tìm node trong mảng theo ID
        GraphNode* findNode(int id);
        // Tìm cạnh giữa u và v
        GraphEdge* findEdge(int u, int v);
    };
}
