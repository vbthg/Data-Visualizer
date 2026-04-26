#include "ShortestPath.h"
#include <algorithm>
#include <sstream>

namespace DS
{
    ShortestPath::ShortestPath() {}

    void ShortestPath::addNode(int id, sf::Vector2f pos)
    {
        m_nodes.push_back({id, pos, std::numeric_limits<float>::infinity(), -1, false});
        // Đồng bộ lại counter để tránh trùng ID khi người dùng mix giữa addEdge và click chuột
        if(id >= m_nextNodeId)
        {
            m_nextNodeId = id + 1;
        }
    }

    void ShortestPath::addEdge(int u, int v, float weight)
    {
        if(!findNode(u)) addNode(u, sf::Vector2f(100.f, 100.f));

        if(!findNode(v))
        {
            GraphNode* nodeU = findNode(u);
            addNode(v, nodeU ? findBestPosition(nodeU->pos) : sf::Vector2f(200.f, 200.f));
        }

        // Bây giờ push_back sẽ cực kỳ an toàn nhờ constructor
        m_edges.push_back(GraphEdge(m_nextEdgeId++, u, v, weight));

        createSnapshot(GUI::Scenario::Success, "Added edge (" + std::to_string(u) + "," + std::to_string(v) + ")", -1);
    }

    void ShortestPath::runDijkstra(int startNodeId, int endNodeId)
    {
        if(m_nodes.empty()) return;

        // Reset dữ liệu trước khi chạy (quan trọng để chạy lại nhiều lần không lỗi)
        for(auto& n : m_nodes)
        {
            n.dist = std::numeric_limits<float>::infinity();
            n.parentId = -1;
            n.finalized = false;
        }
        for(auto& e : m_edges)
        {
            e.visualFill = 0.0f;
            e.visualColor = sf::Color(150, 150, 150, 100);
            e.inPath = false;
        }

        GraphNode* startNode = findNode(startNodeId);
        if(!startNode) return;
        startNode->dist = 0;

        m_timeline->onNewMacroStarted();
        createSnapshot(GUI::Scenario::Processing, "Initialize Dijkstra from source " + std::to_string(startNodeId), 0);

        using pii = std::pair<float, int>;
        std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;
        pq.push({0.0f, startNodeId});

        while(!pq.empty())
        {
            float d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            GraphNode* nodeU = findNode(u);
            if(d > nodeU->dist) continue;

            // ĐÁNH DẤU FINALIZED TRƯỚC (Đúng như em yêu cầu)
            nodeU->finalized = true;

            // Snapshot này sẽ cho thấy đỉnh u chuyển sang màu vàng (Focus)
            // nhưng thực tế nó đã được gán finalized ngầm bên dưới.
            createSnapshot(GUI::Scenario::Processing, "Processing node " + std::to_string(u), 4, u);

            for(auto& edge : m_edges)
            {
                int v = -1;
                if(edge.u == u) v = edge.v;
                else if(edge.v == u) v = edge.u;

                if(v != -1)
                {
                    GraphNode* nodeV = findNode(v);
                    if(nodeV->finalized) continue;

                    createSnapshot(GUI::Scenario::Processing, "Checking edge to neighbor " + std::to_string(v), 7, u, v, edge.id);

                    if(nodeU->dist + edge.weight < nodeV->dist)
                    {
                        nodeV->dist = nodeU->dist + edge.weight;
                        nodeV->parentId = u;
                        pq.push({nodeV->dist, v});

                        edge.visualFill = 1.0f;
                        edge.visualColor = COLOR_RELAX;
                        createSnapshot(GUI::Scenario::Success, "Distance updated!", 11, u, v, edge.id);
                    }
                    else
                    {
                        edge.visualFill = 1.0f;
                        edge.visualColor = sf::Color(255, 69, 58, 80); // Đỏ mờ
                        createSnapshot(GUI::Scenario::Warning, "Not a better path", 9, u, v, edge.id);
                    }
                }
            }
        }

        // SNAPSHOT CUỐI CÙNG: Dọn dẹp highlight (currentU = -1, currentV = -1)
        // Snapshot này sẽ làm đỉnh cuối cùng biến từ Vàng sang Xanh (Finalized)
        createSnapshot(GUI::Scenario::Success, "Dijkstra completed!", 16, -1, -1, -1);

        // Truy vết đường đi nếu có endNode
        if(endNodeId != -1)
        {
            int curr = endNodeId;
            while(curr != -1)
            {
                GraphNode* nodeCurr = findNode(curr);
                int p = nodeCurr->parentId;
                if(p != -1)
                {
                    GraphEdge* e = findEdge(p, curr);
                    if(e) e->inPath = true;
                }
                curr = p;
            }
            createSnapshot(GUI::Scenario::Success, "Shortest path shown in green", 16);
        }
    }

    void ShortestPath::createSnapshot(GUI::Scenario scenario, const std::string& message, int lineIdx, int currentU, int currentV, int relaxingEdgeId, std::vector<std::pair<std::string, std::string>> vars)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        snap->scenario = scenario;
        snap->logMessage = message;
        snap->operationName = "DIJKSTRA";
        snap->macroKey = "dijkstra_shortest_path";
        snap->pseudoCodeLine = lineIdx;
        snap->variableStates = vars;

        // Snapshot Nodes
        for(const auto& n : m_nodes)
        {
            Core::NodeState ns;
            ns.id = n.id;
            ns.position = n.pos;
            ns.value = std::to_string(n.id);

            // Hiển thị khoảng cách ở subText
            if(n.dist == std::numeric_limits<float>::infinity()) ns.subText = "d: INF";
            else ns.subText = "d: " + std::to_string((int)n.dist);

            // Màu sắc theo trạng thái
            if(n.id == currentU) ns.fillColor = COLOR_FOCUS;
            else if(n.id == currentV) ns.fillColor = COLOR_RELAX;
            else if(n.finalized) ns.fillColor = COLOR_SUCCESS;
            else ns.fillColor = COLOR_DEFAULT;

            ns.scale = (n.id == currentU || n.id == currentV) ? 1.2f : 1.0f;
            ns.isDraggable = true;
            snap->nodeStates.push_back(ns);
        }

        // Snapshot Edges
        for(auto& e : m_edges)
        {
            Core::EdgeState es;
            es.startNodeId = e.u;
            es.endNodeId = e.v;
            es.subText = std::to_string((int)e.weight);

            if(e.id == relaxingEdgeId)
            {
                // Cạnh đang được active thì fill theo hướng từ u sang v
                es.baseFillColor = e.visualColor;
                es.fillColor = (scenario == GUI::Scenario::Success) ? COLOR_SUCCESS : (scenario == GUI::Scenario::Warning ? COLOR_ERROR : COLOR_FOCUS);
                es.fillProgress = 1.0f;
                es.fillFromStart = (e.u == currentU);
            }
            else
            {
                // CẬP NHẬT QUAN TRỌNG:
                // Nếu không phải cạnh đang xét, hãy dùng giá trị visual đã lưu từ các bước trước
                es.fillProgress = e.visualFill;
                es.fillColor = e.visualColor;
                // Giữ nguyên hướng fill cũ (có thể lưu thêm fillFromStart vào GraphEdge nếu cần chính xác tuyệt đối)
                es.fillFromStart = true;
            }

            // Nếu nằm trong đường đi cuối cùng (sau khi Reconstruct Path)
            if(e.inPath)
            {
                es.fillColor = COLOR_SUCCESS;
                es.fillProgress = 1.0f;
                es.thickness = 6.0f;
            }

            snap->edgeStates.push_back(es);
        }

        if(m_timeline) m_timeline->addSnapshot(snap);
    }

    // --- CÁC HÀM PHỤ TRỢ (Helper Functions) ---

    GraphNode* ShortestPath::findNode(int id)
    {
        for(auto& n : m_nodes) if(n.id == id) return &n;
        return nullptr;
    }

    GraphEdge* ShortestPath::findEdge(int u, int v)
    {
        for(auto& e : m_edges)
        {
            if((e.u == u && e.v == v) || (e.u == v && e.v == u)) return &e;
        }
        return nullptr;
    }

    void ShortestPath::updateNodePosition(int id, sf::Vector2f newPos)
    {
        GraphNode* n = findNode(id);
        if(n) n->pos = newPos;
    }

    sf::Vector2f ShortestPath::findBestPosition(sf::Vector2f pivotPos)
    {
        float radius = 180.f;
        for(float angle = 0; angle < 360; angle += 45)
        {
            float rad = angle * 3.14159f / 180.f;
            sf::Vector2f testPos = pivotPos + sf::Vector2f(cos(rad) * radius, sin(rad) * radius);
            if(isPositionSafe(testPos, 120.f)) return testPos;
        }
        return pivotPos + sf::Vector2f(radius, 0.f);
    }

    bool ShortestPath::isPositionSafe(sf::Vector2f pos, float minDistance)
    {
        for(const auto& n : m_nodes)
        {
            float dx = n.pos.x - pos.x;
            float dy = n.pos.y - pos.y;
            if(dx*dx + dy*dy < minDistance * minDistance) return false;
        }
        return true;
    }

    std::vector<Command> ShortestPath::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command("Add Edge (u v w)", InputType::String, [this](InputArgs args) {
            std::stringstream ss(args.sVal);
            int u, v; float w;
            if(ss >> u >> v >> w) this->addEdge(u, v, w);
        }));
        cmds.push_back(Command("Run Dijkstra (start [end])", InputType::String, [this](InputArgs args) {
            std::stringstream ss(args.sVal);
            int s, t = -1;
            if(ss >> s) { if(!(ss >> t)) t = -1; this->runDijkstra(s, t); }
        }));
        cmds.push_back(Command("Clear", InputType::None, [this](InputArgs args) {
            this->m_nodes.clear(); this->m_edges.clear();
            this->m_timeline->onNewMacroStarted();
            this->createSnapshot(GUI::Scenario::Success, "Canvas cleared", -1);
        }));
        return cmds;
    }

} // namespace DS
