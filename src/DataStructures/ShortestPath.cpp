#include "ShortestPath.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <queue>
#include <algorithm>

namespace DS
{
    ShortestPath::ShortestPath() {}

    void ShortestPath::addNode(int id, sf::Vector2f pos)
    {
        if(findNode(id)) return;

        sf::Vector2f safePos = pos;
        // Nếu vị trí chỉ định bị đè, tự động tìm vị trí mới quanh đó
        if(!isPositionSafe(pos, 100.0f))
        {
            safePos = findBestPosition(pos);
        }

        m_nodes.push_back({id, safePos, std::numeric_limits<float>::infinity(), -1, false});
        if(id >= m_nextNodeId) m_nextNodeId = id + 1;
    }

    void ShortestPath::addEdge(int u, int v, float weight)
    {
        m_timeline->onNewMacroStarted();
        createSnapshot(GUI::Scenario::Processing, "Add Edge", "Adding connection between " + std::to_string(u) + " and " + std::to_string(v), -1);

        if(!findNode(u)) addNode(u, sf::Vector2f(200.f, 200.f));
        if(!findNode(v)) addNode(v, findBestPosition(findNode(u)->pos));

        m_edges.push_back(GraphEdge(m_nextEdgeId++, u, v, weight));

        createSnapshot(GUI::Scenario::Success, "Edge Added", "Weight: " + std::to_string((int)weight), -1);
        m_timeline->onMacroFinished();
    }

    void ShortestPath::runDijkstra(int startNodeId, int endNodeId)
    {
        if(m_nodes.empty()) return;
        m_timeline->onNewMacroStarted();

        createSnapshot(GUI::Scenario::Processing, "Dijkstra Initializing", "Resetting all nodes to infinity", -1);

        for(auto& n : m_nodes)
        {
            n.dist = std::numeric_limits<float>::infinity();
            n.parentId = -1;
            n.finalized = false;
        }
        for(auto& e : m_edges)
        {
            e.lastFillProgress = 0.0f;
            e.inFinalPath = false;
        }

        GraphNode* startNode = findNode(startNodeId);
        if(!startNode)
        {
            createSnapshot(GUI::Scenario::Error, "Error", "Source node not found!", -1);
            m_timeline->onMacroFinished();
            return;
        }

        startNode->dist = 0;
        createSnapshot(GUI::Scenario::Processing, "Source Found", "Distance[source] = 0", 0, startNodeId);

        using pii = std::pair<float, int>;
        std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;
        pq.push({0.0f, startNodeId});

        while(!pq.empty())
        {
            int u = pq.top().second;
            float d = pq.top().first;
            pq.pop();

            GraphNode* nodeU = findNode(u);
            if(d > nodeU->dist) continue;

            createSnapshot(GUI::Scenario::Processing, "Extract Min", "Considering node " + std::to_string(u), 4, u, -1, -1, {{"u", std::to_string(u)}, {"dist", std::to_string((int)d)}});

            nodeU->finalized = true;
            createSnapshot(GUI::Scenario::Processing, "Finalize", "Node " + std::to_string(u) + " marked as finalized", 6, u);

            // FIX FLICKERING: Gom nhóm các cạnh theo đỉnh kề
            std::map<int, GraphEdge*> bestNeighbors;
            for(auto& edge : m_edges)
            {
                int neighbor = -1;
                if(edge.u == u) neighbor = edge.v;
                else if(edge.v == u) neighbor = edge.u;

                if(neighbor != -1)
                {
                    // Chỉ lấy cạnh ngắn nhất nối tới 'neighbor' này
                    if(bestNeighbors.find(neighbor) == bestNeighbors.end() || edge.weight < bestNeighbors[neighbor]->weight)
                    {
                        bestNeighbors[neighbor] = &edge;
                    }
                }
            }

            // Duyệt qua các hàng xóm dựa trên cạnh tốt nhất đã lọc
            for(auto const& [v, edge] : bestNeighbors)
            {
                GraphNode* nodeV = findNode(v);
                if(nodeV->finalized) continue;

                createSnapshot(GUI::Scenario::Processing, "Relaxing", "Checking path to " + std::to_string(v), 9, u, v, edge->id);

                float newDist = nodeU->dist + edge->weight;
                if(newDist < nodeV->dist)
                {
                    nodeV->dist = newDist;
                    nodeV->parentId = u;
                    pq.push({newDist, v});

                    // Lưu lại trạng thái để các frame sau vẫn thấy cạnh này đã được "duyệt qua"
                    edge->lastFillProgress = 1.0f;
                    edge->lastColor = COLOR_VISITED;

                    createSnapshot(GUI::Scenario::Success, "Relaxed", "Better path found! New dist: " + std::to_string((int)newDist), 11, u, v, edge->id);
                }
                else
                {
                    createSnapshot(GUI::Scenario::Warning, "Ignored", "Not a better path", 9, u, v, edge->id);
                }
            }
        }

        // TRUY VẾT: Nếu có endNodeId, highlight con đường ngắn nhất
        if(endNodeId != -1)
        {
            GraphNode* targetNode = findNode(endNodeId);
            if(targetNode && targetNode->dist != std::numeric_limits<float>::infinity())
            {
                createSnapshot(GUI::Scenario::Processing, "Traceback", "Reconstructing path from target " + std::to_string(endNodeId), -1);

                int curr = endNodeId;
                while(curr != -1)
                {
                    GraphNode* n = findNode(curr);
                    int p = n->parentId;
                    if(p != -1)
                    {
                        GraphEdge* bestE = findBestEdge(p, curr);
                        if(bestE) bestE->inFinalPath = true;
                    }
                    curr = p;
                }
                createSnapshot(GUI::Scenario::Success, "Path Found", "Shortest path traced successfully!", 16);
            }
            else
            {
                createSnapshot(GUI::Scenario::Error, "No Path", "Cannot reach node " + std::to_string(endNodeId), -1);
            }
        }
        else
        {
            createSnapshot(GUI::Scenario::Success, "Done", "Dijkstra completed for all reachable nodes", 16);
        }

        m_timeline->onMacroFinished();
    }

    // Luôn tìm cạnh có trọng số nhỏ nhất để truy vết/hiển thị
    GraphEdge* ShortestPath::findBestEdge(int u, int v)
    {
        GraphEdge* best = nullptr;
        for(auto& e : m_edges)
        {
            if((e.u == u && e.v == v) || (e.u == v && e.v == u))
            {
                if(!best || e.weight < best->weight) best = &e;
            }
        }
        return best;
    }

    void ShortestPath::createSnapshot(GUI::Scenario scenario, const std::string& title, const std::string& subtitle, int lineIdx, int currentU, int currentV, int activeEdgeId, std::vector<std::pair<std::string, std::string>> vars)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        snap->notchData = {scenario, title, subtitle, ""};
        snap->codeData = {"dijkstra_shortest_path", lineIdx, vars};

        for(const auto& n : m_nodes)
        {
            Core::NodeState ns;
            ns.id = n.id;
            ns.position = n.pos;
            ns.value = std::to_string(n.id);
            ns.subText = (n.dist == std::numeric_limits<float>::infinity()) ? "inf" : std::to_string((int)n.dist);
            ns.scale = (n.id == currentU || n.id == currentV) ? 1.15f : 1.0f;
            ns.opacity = 1.0f;
            ns.isDraggable = true;

            if(n.id == currentU) ns.fillColor = COLOR_FOCUS;
            else if(n.id == currentV) ns.fillColor = COLOR_VISITED;
            else if(n.finalized) ns.fillColor = COLOR_SUCCESS;
            else ns.fillColor = COLOR_DEFAULT;

            ns.textColor = (ns.fillColor == COLOR_DEFAULT) ? sf::Color::Black : sf::Color::Black;
            snap->nodeStates.push_back(ns);
        }

        for(const auto& e : m_edges)
        {
            Core::EdgeState es;
            es.startNodeId = e.u; es.endNodeId = e.v;
            es.subText = std::to_string((int)e.weight);
            es.opacity = 1.0f;

            // Cạnh trong đường đi cuối cùng sẽ dày hơn
            es.thickness = e.inFinalPath ? 6.0f : 4.0f;

            if(e.id == activeEdgeId)
            {
                es.isFocused = true;
                es.fillProgress = 1.0f;
                es.fillFromStart = (e.u == currentU); // Lan tỏa từ node đang xét

                if(scenario == GUI::Scenario::Success) es.fillColor = COLOR_SUCCESS;
                else if(scenario == GUI::Scenario::Warning) es.fillColor = COLOR_SKIP;
                else es.fillColor = COLOR_FOCUS;
            }
            else
            {
                es.isFocused = false;
                es.fillProgress = e.lastFillProgress;
                es.fillColor = e.inFinalPath ? COLOR_SUCCESS : e.lastColor;
            }
            snap->edgeStates.push_back(es);
        }
        m_timeline->addSnapshot(snap);
    }

    bool ShortestPath::loadFromFile(const std::string& path)
    {
        m_timeline->onNewMacroStarted();
        // Snapshot 1: Thông báo đang đọc dữ liệu
        createSnapshot(GUI::Scenario::Processing, "Importing data", "Reading file: " + path, -1);

        std::ifstream ifs(path);
        if(!ifs.is_open())
        {
            m_lastError = "Could not open file.";
            createSnapshot(GUI::Scenario::Error, "Load Failed", "File not found: " + path, -1);
            m_timeline->onMacroFinished();
            return false;
        }

        m_nodes.clear();
        m_edges.clear();

        int n, m;
        if(ifs >> n >> m)
        {
            // 1. Tự động sinh n node từ 1 đến n
            // Đặt node đầu tiên ở trung tâm, các node sau sẽ tự tìm vị trí an toàn
            sf::Vector2f startPivot(400.f, 300.f);
            for(int i = 1; i <= n; ++i)
            {
                addNode(i, startPivot);
            }

            // 2. Đọc m cạnh và nối chúng lại
            for(int i = 0; i < m; ++i)
            {
                int u, v;
                float w;
                if(ifs >> u >> v >> w)
                {
                    // Kiểm tra u, v có nằm trong danh sách node vừa tạo không
                    if(findNode(u) && findNode(v))
                    {
                        m_edges.push_back(GraphEdge(m_nextEdgeId++, u, v, w));
                    }
                }
            }
        }

        // Snapshot 2: Thông báo hoàn tất nạp dữ liệu
        createSnapshot(GUI::Scenario::Success, "Import Success",
                       "Generated " + std::to_string(m_nodes.size()) + " nodes and " + std::to_string(m_edges.size()) + " edges", -1);

        m_timeline->onMacroFinished();
        return true;
    }

    // Các hàm helper giữ nguyên logic an toàn tọa độ
    sf::Vector2f ShortestPath::findBestPosition(sf::Vector2f pivotPos)
    {
        float radius = 150.0f;
        for(int circle = 1; circle <= 3; circle++)
        {
            for(float angle = 0; angle < 360.0f; angle += 30.0f)
            {
                float rad = angle * (3.14159f / 180.0f);
                sf::Vector2f testPos = pivotPos + sf::Vector2f(std::cos(rad) * radius * circle, std::sin(rad) * radius * circle);
                if(isPositionSafe(testPos, 100.0f)) return testPos;
            }
        }
        return pivotPos + sf::Vector2f(200, 0);
    }

    bool ShortestPath::isPositionSafe(sf::Vector2f pos, float minDistance)
    {
        for(const auto& n : m_nodes)
        {
            float dx = pos.x - n.pos.x, dy = pos.y - n.pos.y;
            if(dx*dx + dy*dy < minDistance * minDistance) return false;
        }
        return true;
    }

    void ShortestPath::updateNodePosition(int id, sf::Vector2f newPos)
    {
        if(auto n = findNode(id)) n->pos = newPos;
        m_timeline->updateNodePositionInHistory(id, newPos);
    }

    GraphNode* ShortestPath::findNode(int id) { for(auto& n : m_nodes) if(n.id == id) return &n; return nullptr; }

    std::vector<Command> ShortestPath::getCommands()
    {
        std::vector<Command> cmds;
        // Thêm cạnh
        cmds.push_back(Command(L"\uE6D2", "Add Edge (u v w)", InputType::String, [this](InputArgs args) {
            std::stringstream ss(args.sVal); int u, v; float w;
            if(ss >> u >> v >> w) this->addEdge(u, v, w);
        }));

        // Tìm đường đi cụ thể s -> t (Có truy vết)
        cmds.push_back(Command(L"\uE47C", "Path s -> t", InputType::String, [this](InputArgs args) {
            std::stringstream ss(args.sVal); int s, t;
            if(ss >> s >> t) this->runDijkstra(s, t);
        }));

        // Chạy Dijkstra từ nguồn s (Không giới hạn đích)
        cmds.push_back(Command(L"\uEB58", "Dijkstra from s", InputType::String, [this](InputArgs args) {
            std::stringstream ss(args.sVal); int s;
            if(ss >> s) this->runDijkstra(s, -1);
        }));

        cmds.push_back(Command(L"\uEC54", "Clear", InputType::None, [this](InputArgs args) {
            this->m_timeline->onNewMacroStarted();
            createSnapshot(GUI::Scenario::Processing, "Clearing", "Wiping graph data", -1);
            this->m_nodes.clear(); this->m_edges.clear();
            createSnapshot(GUI::Scenario::Success, "Cleared", "Canvas is now empty", -1);
            this->m_timeline->onMacroFinished();
        }));
        return cmds;
    }
}
