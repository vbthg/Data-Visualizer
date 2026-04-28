#pragma once
#include "DataStructure.h"
#include "GUI/NotchEnums.h"
#include <vector>
#include <string>
#include <map>

namespace DS
{
    struct GraphNode
    {
        int id;
        sf::Vector2f pos;
        float dist;
        int parentId;
        bool finalized;
    };

    struct GraphEdge
    {
        int id;
        int u, v;
        float weight;

        float lastFillProgress = 0.0f;
        sf::Color lastColor = sf::Color(150, 150, 150, 100);
        bool inFinalPath = false;

        GraphEdge(int _id, int _u, int _v, float _w)
            : id(_id), u(_u), v(_v), weight(_w) {}
    };

    class ShortestPath : public DataStructure
    {
    private:
        std::vector<GraphNode> m_nodes;
        std::vector<GraphEdge> m_edges;
        int m_nextNodeId = 0;
        int m_nextEdgeId = 0;
        std::string m_lastError = "";

        const sf::Color COLOR_DEFAULT = sf::Color(200, 200, 200);
        const sf::Color COLOR_FOCUS   = sf::Color(255, 212, 59);  // Vàng
        const sf::Color COLOR_SUCCESS = sf::Color(52, 199, 89);   // Xanh lá
        const sf::Color COLOR_VISITED = sf::Color(0, 122, 255);   // Xanh dương
        const sf::Color COLOR_SKIP    = sf::Color(255, 69, 58);   // Đỏ

    public:
        ShortestPath();

        void addNode(int id, sf::Vector2f pos);
        void addEdge(int u, int v, float weight);
        void runDijkstra(int startNodeId, int endNodeId = -1);

        virtual bool loadFromFile(const std::string& path) override;
        virtual std::string getLastError() const override { return m_lastError; }
        virtual void updateNodePosition(int id, sf::Vector2f newPos) override;
        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "DIJKSTRA SHORTEST PATH"; }

    private:
        void createSnapshot(GUI::Scenario scenario, const std::string& title, const std::string& subtitle,
                            int lineIdx, int currentU = -1, int currentV = -1, int activeEdgeId = -1,
                            std::vector<std::pair<std::string, std::string>> vars = {});

        GraphNode* findNode(int id);
        // Cập nhật hàm này để luôn lấy cạnh có trọng số nhỏ nhất giữa u và v
        GraphEdge* findBestEdge(int u, int v);
        sf::Vector2f findBestPosition(sf::Vector2f pivotPos);
        bool isPositionSafe(sf::Vector2f pos, float minDistance);
    };
}
