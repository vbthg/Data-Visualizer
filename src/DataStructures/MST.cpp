#include "MST.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

namespace DS
{
    MST::MST() {}

    bool MST::isPositionSafe(sf::Vector2f pos, float minDistance)
    {
        for(const auto& node : m_nodes)
        {
            float dx = pos.x - node.pos.x;
            float dy = pos.y - node.pos.y;
            if(dx*dx + dy*dy < minDistance * minDistance) return false;
        }
        return true;
    }

    sf::Vector2f MST::findBestPosition(sf::Vector2f pivotPos)
    {
        float radius = 150.0f;
        float angleStep = 30.0f;
        float currentRadius = radius;

        for(int circle = 0; circle < 3; circle++)
        {
            for(float angle = 0; angle < 360.0f; angle += angleStep)
            {
                float rad = angle * (3.14159f / 180.0f);
                sf::Vector2f testPos = { pivotPos.x + currentRadius * cos(rad), pivotPos.y + currentRadius * sin(rad) };
                if(isPositionSafe(testPos, 100.0f)) return testPos;
            }
            currentRadius += 100.0f;
        }
        return pivotPos + sf::Vector2f(150.f, 0.f);
    }

    bool MST::loadFromFile(const std::string& path)
    {
        m_timeline->onNewMacroStarted();
        createSnapshot(GUI::Scenario::Processing, "Importing data", "Reading: " + path, -1, {});

        std::ifstream file(path);
        if(!file.is_open())
        {
            createSnapshot(GUI::Scenario::Error, "Import Failed", "Could not open file", -1, {});
            m_timeline->onMacroFinished();
            return false;
        }

        m_nodes.clear();
        m_edges.clear();
        m_nextNodeId = 0;
        m_nextEdgeId = 0;

        int n, m;
        if(!(file >> n >> m))
        {
            createSnapshot(GUI::Scenario::Error, "Import Failed", "Invalid format", -1, {});
            m_timeline->onMacroFinished();
            return false;
        }

        sf::Vector2f center(960.f, 540.f);
        for(int i = 1; i <= n; ++i)
        {
            // Node đầu tiên ưu tiên tâm, các node sau xoắn ốc quanh node trước đó hoặc quanh tâm
            sf::Vector2f pivot = m_nodes.empty() ? center : m_nodes.back().pos;
            sf::Vector2f safePos = isPositionSafe(pivot, 100.0f) ? pivot : findBestPosition(pivot);
            m_nodes.push_back({i, safePos});
        }

        for(int i = 0; i < m; ++i)
        {
            int u, v; float w;
            file >> u >> v >> w;
            m_edges.push_back({m_nextEdgeId++, u, v, w});
        }
        m_nextNodeId = n;

        createSnapshot(GUI::Scenario::Success, "Import Success", "Loaded " + std::to_string(n) + " nodes safely", -1, {});
        m_timeline->onMacroFinished();
        return true;
    }

    void MST::addEdge(int u, int v, float weight)
    {
        std::string macroTitle = "Insert Edge(" + std::to_string(u) + "," + std::to_string(v) + ")";
        m_timeline->onNewMacroStarted();

        // --- Snapshot 1: Base (Preparing) ---
        createSnapshot(GUI::Scenario::Processing, macroTitle, "Finding safe positions for nodes...", -1, {});

        auto findNode = [this](int id) -> Node* {
            for(auto& n : m_nodes) if(n.id == id) return &n;
            return nullptr;
        };

        Node* nodeU = findNode(u);
        Node* nodeV = findNode(v);
        sf::Vector2f center(960.f, 540.f);

        // Trường hợp 1: Cả 2 đều chưa có
        if(!nodeU && !nodeV)
        {
            // Tìm chỗ cho U quanh tâm, nếu tâm trống thì lấy tâm, không thì xoắn ốc
            sf::Vector2f safeU = isPositionSafe(center, 100.0f) ? center : findBestPosition(center);
            m_nodes.push_back({u, safeU});
            // Sau đó tìm chỗ cho V quanh U
            m_nodes.push_back({v, findBestPosition(safeU)});
        }
        // Trường hợp 2: U có rồi, V chưa có
        else if(nodeU && !nodeV)
        {
            m_nodes.push_back({v, findBestPosition(nodeU->pos)});
        }
        // Trường hợp 3: V có rồi, U chưa có
        else if(!nodeU && nodeV)
        {
            m_nodes.push_back({u, findBestPosition(nodeV->pos)});
        }

        m_edges.push_back({m_nextEdgeId++, u, v, weight});
        m_nextNodeId = std::max({m_nextNodeId, u + 1, v + 1});

        // --- Snapshot 2: Hoàn thành (Final) ---
        createSnapshot(GUI::Scenario::Success, macroTitle, "Edge added. All nodes placed safely.", -1, {});

        m_timeline->onMacroFinished();
    }

    void MST::runKruskal()
    {
        if(m_nodes.empty())
        {
            return;
        }
        m_timeline->onNewMacroStarted();

        // --- Bước 1: Khởi tạo & Sắp xếp ---
        createSnapshot(GUI::Scenario::Processing, "KRUSKAL MST", "Step 1: Sorting edges by weight", 0, {});

        std::vector<Edge> sortedEdges = m_edges;
        std::sort(sortedEdges.begin(), sortedEdges.end(), [](const Edge& a, const Edge& b)
        {
            return a.weight < b.weight;
        });

        DSU dsu(m_nextNodeId + 1);
        std::vector<int> mstEdgeIds;

        // --- Bước 2: Duyệt và Hợp nhất ---
        for(const auto& edge : sortedEdges)
        {
            std::vector<std::pair<std::string, std::string>> vars = {
                {"u", std::to_string(edge.u)}, {"v", std::to_string(edge.v)}, {"w", std::to_string((int)edge.weight)}
            };

            int rootU = dsu.find(edge.u);
            int rootV = dsu.find(edge.v);
            int sourceNodeId = (dsu.size[rootU] >= dsu.size[rootV]) ? edge.u : edge.v;

            createSnapshot(GUI::Scenario::Processing, "KRUSKAL MST", "Checking edge (" + std::to_string(edge.u) + "," + std::to_string(edge.v) + ")", 4, mstEdgeIds, edge.id, sourceNodeId, -1, vars);

            if(rootU != rootV)
            {
                dsu.unite(rootU, rootV);
                mstEdgeIds.push_back(edge.id);
                createSnapshot(GUI::Scenario::Success, "KRUSKAL MST", "Accepted! Components connected.", 8, mstEdgeIds, edge.id, sourceNodeId, -1, vars);
            }
            else
            {
                createSnapshot(GUI::Scenario::Warning, "KRUSKAL MST", "Rejected! Cycle detected.", 12, mstEdgeIds, -1, sourceNodeId, edge.id, vars);
            }
        }

        // --- Bước 3: Kiểm tra tính liên thông (KẾT THÚC) ---
        // Điều kiện tồn tại MST: số cạnh = V - 1
        bool isConnected = (mstEdgeIds.size() == m_nodes.size() - 1);

        if(isConnected)
        {
            createSnapshot(GUI::Scenario::Success, "MST FOUND", "All nodes connected. Total edges: " + std::to_string(mstEdgeIds.size()), 15, mstEdgeIds);
        }
        else
        {
            // Nếu không đủ cạnh, ta báo lỗi hoặc cảnh báo
            std::string failMsg = "Graph is disconnected! Found " + std::to_string(mstEdgeIds.size()) + " edges instead of " + std::to_string(m_nodes.size() - 1);
            createSnapshot(GUI::Scenario::Error, "MST NOT FOUND", failMsg, 15, mstEdgeIds);
        }

        m_timeline->onMacroFinished();
    }

    void MST::updateNodePosition(int id, sf::Vector2f newPos)
    {
        for(auto& n : m_nodes)
        {
            if(n.id == id)
            {
                n.pos = newPos;
                if(m_timeline) m_timeline->updateNodePositionInHistory(id, newPos);
                break;
            }
        }
    }

    void MST::createSnapshot(GUI::Scenario scenario,
                             const std::string& title,
                             const std::string& subTitle,
                             int lineIdx,
                             const std::vector<int>& mstEdgeIds,
                             int currentEdgeId,
                             int sourceNodeId,
                             int rejectedEdgeId,
                             std::vector<std::pair<std::string, std::string>> vars,
                             bool resetProgress)
    {
        auto snap = std::make_shared<Core::ISnapshot>();

        // 1. NotchContext
        snap->notchData.scenario = scenario;
        snap->notchData.title = title;
        snap->notchData.subtitle = subTitle;
        snap->notchData.iconCode = ""; // Để trống như yêu cầu

        // 2. CodeContext
        snap->codeData.macroKey = "kruskal_mst";
        snap->codeData.pseudoCodeLine = lineIdx;
        snap->codeData.variableStates = vars;

        // 1. Lấy snapshot cuối cùng trong timeline để truy vấn màu cũ
        std::shared_ptr<Core::ISnapshot> lastSnap = nullptr;
        if(!resetProgress && m_timeline && m_timeline->getCount() > 0)
        {
            lastSnap = m_timeline->getSnapshot(m_timeline->getCount() - 1);
        }

        // 3. Đóng gói Nodes
        for(const auto& n : m_nodes)
        {
            Core::NodeState ns;
            ns.id = n.id;
            ns.position = n.pos;
            ns.value = std::to_string(n.id);
            ns.fillColor = sf::Color::White;
            ns.outlineColor = sf::Color(200, 200, 200);
            ns.isDraggable = true; // Yêu cầu: Đồ thị luôn bật draggable
            snap->nodeStates.push_back(ns);
        }

        // 4. Đóng gói Edges
        for(const auto& e : m_edges)
        {
            Core::EdgeState es;
            es.startNodeId = e.u;
            es.endNodeId = e.v;
            es.subText = std::to_string((int)e.weight);

            // Lấy trạng thái cũ của cạnh này
            float prevFillProgress = 0.0f;
            sf::Color prevFillColor = sf::Color(150, 150, 150);

            if(lastSnap)
            {
                auto it = std::find_if(lastSnap->edgeStates.begin(), lastSnap->edgeStates.end(),
                    [&](const auto& prev) { return prev.startNodeId == e.u && prev.endNodeId == e.v; });

                if(it != lastSnap->edgeStates.end())
                {
                    prevFillProgress = it->fillProgress;
                    prevFillColor = it->fillColor;
                }
            }

            bool isMST = std::find(mstEdgeIds.begin(), mstEdgeIds.end(), e.id) != mstEdgeIds.end();

            if(isMST)
            {
                es.fillColor = sf::Color(52, 199, 89); // Apple Green
                es.baseFillColor = prevFillColor; //
                es.fillProgress = 1.0f;
                es.thickness = 5.0f;
            }
            else if(e.id == currentEdgeId)
            {
                es.fillColor = sf::Color(255, 214, 10); // Apple Yellow
                es.baseFillColor = prevFillColor;
                es.fillProgress = 1.0f;
                es.fillFromStart = (sourceNodeId == e.u);
//                es.isPulsing = true;
            }
            else if(e.id == rejectedEdgeId)
            {
                es.fillColor = sf::Color(255, 69, 58); // Màu đỏ cảnh báo
                es.baseFillColor = prevFillColor;
                es.fillProgress = 1.0f;
                es.fillFromStart = (sourceNodeId == e.u);
                es.opacity = 0.6f;
            }
            else
            {
                es.fillProgress = prevFillProgress;
                es.baseFillColor = prevFillColor;
                es.fillColor = sf::Color(150, 150, 150);
                es.fillColor = prevFillColor; // Không đổi màu nữa
                es.opacity = (prevFillProgress > 0.0f) ? 0.1f : 0.6f; // Mờ đi nếu là cạnh bị loại
            }
            snap->edgeStates.push_back(es);
        }

        if(m_timeline) m_timeline->addSnapshot(snap);
    }

    void MST::clear()
    {
        if(!m_timeline) return;

        m_timeline->onNewMacroStarted();

        // Snapshot Base: Bắt đầu dọn dẹp
        createSnapshot(GUI::Scenario::Processing, "Clear", "Preparing to clear internal data...", -1, {});

        m_nodes.clear();
        m_edges.clear();
        m_nextNodeId = 0;
        m_nextEdgeId = 0;

        // Snapshot Final: Canvas trống
        createSnapshot(GUI::Scenario::Success, "Clear", "Internal structure reset. Canvas is blank.", -1, {}, true);

        m_timeline->onMacroFinished();
    }

    std::vector<Command> MST::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command(L"\uE6D2", "Add Edge (u v w)", InputType::String, [this](InputArgs args)
        {
            std::stringstream ss(args.sVal);
            int u, v; float w;
            if(ss >> u >> v >> w) this->addEdge(u, v, w);
        }));
        cmds.push_back(Command(L"\uE654", "Run Kruskal", InputType::None, [this](InputArgs args)
        {
            this->runKruskal();
        }));
        cmds.push_back(Command(L"\uEC54", "Clear", InputType::None, [this](InputArgs args)
        {
            this->clear();
        }));
        return cmds;
    }
}
