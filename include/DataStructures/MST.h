#pragma once
#include "DataStructure.h"
#include "ISnapshot.h"
#include <vector>
#include <string>

namespace DS
{
    class MST : public DataStructure
    {
    private:
        struct Edge
        {
            int id;
            int u, v;
            float weight;
        };

        struct Node
        {
            int id;
            sf::Vector2f pos;
        };

        struct DSU
        {
            std::vector<int> parent;
            std::vector<int> size;
            DSU(int n)
            {
                parent.resize(n);
                size.assign(n, 1);
                for(int i = 0; i < n; ++i) parent[i] = i;
            }
            int find(int i)
            {
                if(parent[i] == i) return i;
                return parent[i] = find(parent[i]);
            }
            bool unite(int i, int j)
            {
                int root_i = find(i);
                int root_j = find(j);
                if(root_i != root_j)
                {
                    if(size[root_i] < size[root_j]) std::swap(root_i, root_j);
                    parent[root_j] = root_i;
                    size[root_i] += size[root_j];
                    return true;
                }
                return false;
            }
        };

        std::vector<Node> m_nodes;
        std::vector<Edge> m_edges;
        int m_nextNodeId = 0;
        int m_nextEdgeId = 0;

        bool isPositionSafe(sf::Vector2f pos, float minDistance);
        sf::Vector2f findBestPosition(sf::Vector2f pivotPos);

    public:
        MST();
        virtual ~MST() = default;

        std::vector<Command> getCommands() override;
        std::string getName() const override { return "Kruskal's MST"; }

        bool loadFromFile(const std::string& path) override;
        void addEdge(int u, int v, float weight);
        void runKruskal();
        void clear();

        void updateNodePosition(int id, sf::Vector2f newPos) override;

    private:
        void createSnapshot(GUI::Scenario scenario,
                           const std::string& title,
                           const std::string& subTitle,
                           int lineIdx,
                           const std::vector<int>& mstEdgeIds,
                           int currentEdgeId = -1,
                           int sourceNodeId = -1,
                           int rejectedEdgeId = -1,
                           std::vector<std::pair<std::string, std::string>> vars = {},
                           bool resetProgress = false);
    };
}
