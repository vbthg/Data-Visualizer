#include "MST.h"
#include <algorithm>
#include <sstream>

namespace DS
{
    MST::MST() {}

    void MST::addNode(sf::Vector2f pos)
    {
        m_nodes.push_back({m_nextNodeId++, pos});
    }

    bool MST::isPositionSafe(sf::Vector2f pos, float minDistance)
    {
        for(const auto& node : m_nodes)
        {
            float dx = pos.x - node.pos.x;
            float dy = pos.y - node.pos.y;
            float distSq = dx*dx + dy*dy;

            if(distSq < minDistance * minDistance)
            {
                return false; // Quá gần, không an toàn
            }
        }
        return true;
    }

    sf::Vector2f MST::findBestPosition(sf::Vector2f pivotPos)
    {
        float radius = 150.0f; // Khoảng cách lý tưởng giữa 2 node
        float angleStep = 30.0f; // Mỗi lần thử sẽ xoay 30 độ
        float currentRadius = radius;

        // Thử tối đa 3 vòng xoắn ốc để tìm chỗ trống
        for(int circle = 0; circle < 3; circle++)
        {
            for(float angle = 0; angle < 360.0f; angle += angleStep)
            {
                float rad = angle * (3.14159f / 180.0f);
                sf::Vector2f testPos = {
                    pivotPos.x + currentRadius * cos(rad),
                    pivotPos.y + currentRadius * sin(rad)
                };

                if(isPositionSafe(testPos, 100.0f))
                {
                    return testPos;
                }
            }
            currentRadius += 100.0f; // Nới rộng vòng tìm kiếm nếu vòng cũ đã chật
        }

        return pivotPos + sf::Vector2f(150.f, 0.f); // Bí quá thì đặt đại bên phải
    }

    void MST::addEdge(int u, int v, float weight)
    {
        auto findNode = [this](int id) -> Node* {
            for(auto& n : m_nodes)
            {
                if(n.id == id) return &n;
            }
            return nullptr;
        };

        Node* nodeU = findNode(u);
        Node* nodeV = findNode(v);

        // Trường hợp 1: Cả 2 đều chưa có -> Đặt U ở trung tâm, V tìm quanh U
        if(!nodeU && !nodeV)
        {
            sf::Vector2f center(960.f, 540.f);
            m_nodes.push_back({u, center});
            m_nodes.push_back({v, findBestPosition(center)});
        }
        // Trường hợp 2: U có rồi, V chưa có -> Tìm chỗ quanh U cho V
        else if(nodeU && !nodeV)
        {
            m_nodes.push_back({v, findBestPosition(nodeU->pos)});
        }
        // Trường hợp 3: V có rồi, U chưa có -> Tìm chỗ quanh V cho U
        else if(!nodeU && nodeV)
        {
            m_nodes.push_back({u, findBestPosition(nodeV->pos)});
        }

        // Sau khi đảm bảo các node đã tồn tại, thêm cạnh như bình thường
        m_edges.push_back({m_nextEdgeId++, u, v, weight});

        // Đồng bộ ID để các thao tác thủ công không bị trùng
        m_nextNodeId = std::max({m_nextNodeId, u + 1, v + 1});

        // Notch thông báo nhẹ nhàng
        createSnapshot(GUI::Scenario::Success, "Edge added. Nodes auto-arranged.", -1, {});
    }

    void MST::runKruskal()
    {
        if(m_edges.empty()) return;

        // Bắt đầu một Macro mới cho Timeline
        m_timeline->onNewMacroStarted();

        // Snapshot này ép toàn bộ cạnh về trạng thái xám, progress = 0
        createSnapshot(GUI::Scenario::Processing, "Resetting visual state...", -1, {}, -1, -1, -1, {}, true);

        // 1. Khởi tạo: Notch hiện trạng thái Processing (với Waveform)
        createSnapshot(GUI::Scenario::Processing, "Initializing Kruskal's algorithm", 0, {});

        // Sắp xếp các cạnh theo trọng số (Sort edges by weight)
        std::vector<Edge> sortedEdges = m_edges;
        std::sort(sortedEdges.begin(), sortedEdges.end(), [](const Edge& a, const Edge& b){
            return a.weight < b.weight;
        });

        createSnapshot(GUI::Scenario::Processing, "Edges sorted by weight", 1, {}, -1, -1, -1, {{"Edges", std::to_string(m_edges.size())}});

        DSU dsu(m_nextNodeId);
        std::vector<int> mstEdgeIds;

        for(const auto& edge : sortedEdges)
        {
            std::string edgeLabel = "(" + std::to_string(edge.u) + "," + std::to_string(edge.v) + ")";

//            // Bước: Đang xét cạnh (Highlight cạnh hiện tại bằng hiệu ứng Pulse)
//            createSnapshot(GUI::Scenario::Processing, "Checking edge " + edgeLabel, 4,
//                           mstEdgeIds, edge.id, -1,
//                           {{"u", std::to_string(edge.u)}, {"v", std::to_string(edge.v)}, {"w", std::to_string((int)edge.weight)}});

            int rootU = dsu.find(edge.u);
            int rootV = dsu.find(edge.v);

            // Xác định đỉnh nguồn dựa trên kích thước thành phần liên thông
            // Nếu u thuộc tập lớn hơn, màu sẽ đổ từ u sang v
            int sourceNodeId = (dsu.size[rootU] >= dsu.size[rootV]) ? edge.u : edge.v;

//            // Snapshot 1: Bắt đầu đổ màu (Progress 0 -> 1 sẽ diễn ra mượt nhờ Timeline)
//            createSnapshot(GUI::Scenario::Processing, "Checking edge...", 4,
//                           mstEdgeIds, edge.id, sourceNodeId);

            if(rootU != rootV)
            {
                dsu.unite(rootU, rootV);
                mstEdgeIds.push_back(edge.id);
                // Snapshot 2: Chấp nhận (Màu xanh hiện ra tại vị trí đã fill xong)
                createSnapshot(GUI::Scenario::Success, "Accepted!", 8, mstEdgeIds, edge.id, sourceNodeId);
            }
            else
            {
                // Snapshot 2: Từ chối (Màu đỏ hiện ra)
                createSnapshot(GUI::Scenario::Warning, "Cycle!", 12, mstEdgeIds, -1, sourceNodeId, edge.id);
            }
        }

        // KẾT THÚC: Notch hiện Success và tự thu hồi
        createSnapshot(GUI::Scenario::Success, "MST Construction Complete", 15, mstEdgeIds);
    }

    void MST::updateNodePosition(int id, sf::Vector2f newPos)
    {
        for(auto& n : m_nodes)
        {
            if(n.id == id)
            {
                n.pos = newPos;
                break;
            }
        }
    }

    void MST::createSnapshot(GUI::Scenario scenario,
                         const std::string& message,
                         int lineIdx,
                         const std::vector<int>& mstEdgeIds,
                         int currentEdgeId,
                         int sourceNodeId,
                         int rejectedEdgeId,
                         std::vector<std::pair<std::string, std::string>> vars,
                         bool resetProgress)
    {
        auto snap = std::make_shared<Core::ISnapshot>();

        // 1. Lấy snapshot cuối cùng trong timeline để truy vấn màu cũ
        std::shared_ptr<Core::ISnapshot> lastSnap = nullptr;
        if(!resetProgress && m_timeline && m_timeline->getCount() > 0)
        {
            lastSnap = m_timeline->getSnapshot(m_timeline->getCount() - 1);
        }

        // Thiết lập dữ liệu cho Dynamic Notch và PseudoCodeBox
        snap->scenario = scenario;
        snap->logMessage = message;
        snap->operationName = "KRUSKAL MST";
        snap->macroKey = "kruskal_mst";
        snap->pseudoCodeLine = lineIdx;
        snap->variableStates = vars;

        // 1. Snapshot các đỉnh (Nodes)
        for(const auto& n : m_nodes)
        {
            Core::NodeState ns;
            ns.id = n.id;
            ns.position = n.pos;
            ns.value = std::to_string(n.id);
            ns.fillColor = sf::Color::White;
            ns.outlineColor = sf::Color(200, 200, 200);
            ns.isDraggable = true;
            snap->nodeStates.push_back(ns);
        }

        // 2. Snapshot các cạnh (Edges) với logic Color Filling
for(const auto& e : m_edges)
    {
        Core::EdgeState es;
        es.startNodeId = e.u;
        es.endNodeId = e.v;

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

        if(isMST || e.id == currentEdgeId)
        {
            es.fillColor = isMST ? sf::Color(52, 199, 89) : sf::Color(255, 212, 59);
            es.baseFillColor = prevFillColor; // Nền là màu của frame trước
            es.fillProgress = 1.0f; // Luôn giữ 1.0 để không bị co lại
            es.fillFromStart = (sourceNodeId == e.u);
            es.opacity = 1.0f;
        }
        else if(e.id == rejectedEdgeId)
        {
            es.fillColor = sf::Color(255, 69, 58); // Màu đỏ cảnh báo
            es.baseFillColor = prevFillColor;
            es.fillProgress = 1.0f;
            es.fillFromStart = (sourceNodeId == e.u);
            es.opacity = 0.8f;
        }
        else
        {
            // ĐÂY LÀ CHỖ CẦN SỬA:
            // Thay vì reset về 0, ta giữ nguyên progress của frame trước
            es.fillProgress = prevFillProgress;
            es.baseFillColor = prevFillColor;
            es.fillColor = sf::Color(150, 150, 150);
            es.fillColor = prevFillColor; // Không đổi màu nữa
            es.opacity = (prevFillProgress > 0.0f) ? 0.3f : 0.1f; // Mờ đi nếu là cạnh bị loại
        }
        es.opacity = 1.f;

        snap->edgeStates.push_back(es);
    }

        if(m_timeline)
        {
            m_timeline->addSnapshot(snap);
        }
    }

void MST::clear()
{
    if(!m_timeline)
    {
        return;
    }

    // 1. Xóa sạch dữ liệu logic
    m_nodes.clear();
    m_edges.clear();
    m_nextNodeId = 0;
    m_nextEdgeId = 0;

    // 2. Thông báo bắt đầu một chuỗi hành động mới (Macro)
    m_timeline->onNewMacroStarted();

    // 3. CHỤP SNAPSHOT TRẮNG:
    // - mstEdgeIds gửi vào là vector rỗng {}
    // - resetProgress = true để đảm bảo không kế thừa bất kỳ màu sắc nào từ quá khứ
    // - Scenario::Success để Notch hiện thông báo "đã dọn dẹp xong"
    createSnapshot(GUI::Scenario::Success, "Canvas cleared", -1, {}, -1, -1, -1, {}, true);
}

    std::vector<Command> MST::getCommands()
    {
        std::vector<Command> cmds;
        // Mockup: Trong thực tế bạn sẽ thêm command để click tạo Node/Edge

        // Nút bấm thêm cạnh: Người dùng nhập "0 1 10" (u v w)
        cmds.push_back(Command("Add Edge (u v w)", InputType::String, [this](InputArgs args)
        {
            std::stringstream ss(args.sVal);
            int u, v;
            float w;

            // Nếu parse thành công 3 giá trị thì mới thực hiện
            if(ss >> u >> v >> w)
            {
                this->addEdge(u, v, w);
            }
        }));

        cmds.push_back(Command(L"Run Kruskal", InputType::None, [this](InputArgs args) {
            this->runKruskal();
        }));
        cmds.push_back(Command(L"Clear", InputType::None, [this](InputArgs args) {
            this->clear();
        }));
        return cmds;
    }
}
