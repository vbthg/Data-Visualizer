#include "Heap.h"
#include "ResourceManager.h"
#include "DynamicIsland.h"
#include "Interpolator.h"
#include "TreeLayoutUtils.h"
#include <cmath>
#include <algorithm>

namespace DS
{
    Heap::Heap()
    {
        m_font = &ResourceManager::getInstance().getFont("assets/fonts/font.ttf");
        m_idCounter = 0;
        m_currentMacroStep = 0;
        m_island = nullptr;
    }

    Heap::~Heap()
    {
        clear();
        for(auto& pair : m_nodeMap)
        {
            delete pair.second;
        }
        m_nodeMap.clear();

        for(auto& pair : m_edgeMap)
        {
            delete pair.second;
        }
        m_edgeMap.clear();
    }

    void Heap::bindDynamicIsland(GUI::DynamicIsland* island)
    {
        m_island = island;
    }

    std::string Heap::getName() const
    {
        return "Max Heap";
    }

    std::vector<DS::Command> Heap::getCommands()
    {
        std::vector<DS::Command> cmds;

        cmds.push_back(DS::Command("Insert", DS::InputType::Integer, [this](DS::InputArgs args)
        {
            this->insert(args.iVal1);
        }));

        cmds.push_back(DS::Command("Extract Max", DS::InputType::None, [this](DS::InputArgs)
        {
            this->extractMax();
        }));

        cmds.push_back(DS::Command("Clear", DS::InputType::None, [this](DS::InputArgs)
        {
            this->clear();
        }));

        return cmds;
    }

    // --- CÁC HÀM HỖ TRỢ KIẾN TRÚC TIMELINE ---

    sf::Vector2f Heap::getTheoreticalPosition(int index)
    {
        // Quy ước: Tâm cây là 0, gốc (Root) bắt đầu từ Y = 0
        float centerX = 0.0f;
        float startY = 0.0f;

        if (index == 0) return sf::Vector2f(centerX, startY);

        int level = std::log2(index + 1);
        int nodesInLevel = std::pow(2, level);
        int positionInLevel = index - (nodesInLevel - 1);

        float ySpacing = 120.0f; // Khoảng cách giữa các tầng
        float xSpacing = 60.0f * std::pow(2, 5 - level);

        float startX = centerX - ((nodesInLevel - 1) * xSpacing) / 2.0f;
        float x = startX + positionInLevel * xSpacing;
        float y = startY + level * ySpacing;

        return sf::Vector2f(x, y);
    }

    void Heap::captureSnapshot(const std::string& logMessage, Utils::Core::TransitionType transition, int macroStepID)
    {
        Utils::Core::Snapshot snap;
        snap.logMessage = logMessage;
        snap.transitionFromPrevious = transition;
        snap.macroStepID = macroStepID;

        // 1. TÍNH TOÁN TOÀN BỘ VỊ TRÍ MỚI CHO SNAPSHOT NÀY
        // rootPos bạn có thể để {0, 0} hoặc tọa độ bạn muốn
        auto calculatedPositions = Utils::TreeLayoutUtils::calculateHeapPositions(m_data.size(), {0.f, 0.f});

        // 1. Chụp lại toàn bộ trạng thái Node
        for (int i = 0; i < (int)m_data.size(); ++i)
        {
            Utils::Core::NodeState state;
            state.id = m_data[i].id;
            state.value = std::to_string(m_data[i].value);
            state.position = calculatedPositions[i];
//            state.position = getTheoreticalPosition(i);

            // Highlight root hoặc node đang xét nếu cần thiết có thể thêm logic đổi màu ở đây
            state.fillColor = sf::Color::White;

            snap.nodeStates[state.id] = state;
        }

        // 2. Chụp lại toàn bộ trạng thái Edge
        for (int i = 0; i < (int)m_data.size(); ++i)
        {
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < (int)m_data.size())
            {
                Utils::Core::EdgeState edge;
                edge.startNodeId = m_data[i].id;
                edge.endNodeId = m_data[left].id;
                snap.edgeStates[{edge.startNodeId, edge.endNodeId}] = edge;
            }
            if (right < (int)m_data.size())
            {
                Utils::Core::EdgeState edge;
                edge.startNodeId = m_data[i].id;
                edge.endNodeId = m_data[right].id;
                snap.edgeStates[{edge.startNodeId, edge.endNodeId}] = edge;
            }
        }

        m_timeline.addSnapshot(snap);
    }

    // --- CÁC THAO TÁC LOGIC THUẬT TOÁN ---

    void Heap::insert(int value)
    {
        // Nếu đang ở quá khứ, cắt bỏ tương lai để rẽ nhánh lịch sử
        m_timeline.truncateAtCurrent();

        m_currentMacroStep++;

        HeapElement newElem;
        newElem.value = value;
        newElem.id = m_idCounter++;
        m_data.push_back(newElem);

        captureSnapshot("Inserted value: " + std::to_string(value), Utils::Core::TransitionType::Linear, m_currentMacroStep);

        heapifyUp(m_data.size() - 1, m_currentMacroStep);

        m_timeline.play();
    }

    void Heap::extractMax()
    {
        m_timeline.truncateAtCurrent();

        if (m_data.empty())
        {
            return;
        }

        m_currentMacroStep++;

        if (m_data.size() == 1)
        {
            m_data.clear();
            captureSnapshot("Extracted the last element", Utils::Core::TransitionType::FadeOnly, m_currentMacroStep);
            m_timeline.play();
            return;
        }

        // Hoán vị gốc và phần tử cuối
        std::swap(m_data[0], m_data.back());
        captureSnapshot("Swapping max with last element", Utils::Core::TransitionType::Orbital, m_currentMacroStep);

        // Xóa phần tử cuối (đã là max)
        m_data.pop_back();
        captureSnapshot("Extracted max element", Utils::Core::TransitionType::Linear, m_currentMacroStep);

        heapifyDown(0, m_currentMacroStep);

        m_timeline.play();
    }

    void Heap::clear()
    {
        m_timeline.truncateAtCurrent();
        m_currentMacroStep++;

        m_data.clear();
        captureSnapshot("Cleared heap", Utils::Core::TransitionType::FadeOnly, m_currentMacroStep);

        m_timeline.play();
    }

    void Heap::heapifyUp(int index, int macroStepID)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;

            if (m_data[index].value > m_data[parent].value)
            {
                std::swap(m_data[index], m_data[parent]);

                // Hiệu ứng xoay tròn khi hoán vị
                captureSnapshot("Swapping " + std::to_string(m_data[index].value) + " and " + std::to_string(m_data[parent].value),
                                Utils::Core::TransitionType::Orbital, macroStepID);

                index = parent;
            }
            else
            {
                break;
            }
        }
    }

    void Heap::heapifyDown(int index, int macroStepID)
    {
        int size = m_data.size();

        while (true)
        {
            int largest = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < size && m_data[left].value > m_data[largest].value)
            {
                largest = left;
            }

            if (right < size && m_data[right].value > m_data[largest].value)
            {
                largest = right;
            }

            if (largest != index)
            {
                std::swap(m_data[index], m_data[largest]);

                captureSnapshot("Swapping down " + std::to_string(m_data[index].value) + " and " + std::to_string(m_data[largest].value),
                                Utils::Core::TransitionType::Orbital, macroStepID);

                index = largest;
            }
            else
            {
                break;
            }
        }
    }

    // --- VÒNG LẶP RENDER & UPDATE CỦA ENGINE ---

    void Heap::update(float dt)
    {
        // Timeline cập nhật thời gian
        m_timeline.update(dt);

        // Hiển thị message lên Dynamic Notch nếu cần
        if (m_island && !m_timeline.isEmpty())
        {
            std::string currentMsg = m_timeline.getSnapshot(m_timeline.getCurrentIdx()).logMessage;
            if (!currentMsg.empty())
            {
                m_island->showMessage(currentMsg);
            }
        }

        // Cập nhật vật lý lò xo cho tất cả Node hiện có trên màn hình
        for (auto& pair : m_nodeMap)
        {
            pair.second->update(dt);
        }

        // Cập nhật vật lý lò xo cho tất cả Edge hiện có
        for (auto& pair : m_edgeMap)
        {
            pair.second->update(dt);
        }
    }

    void Heap::draw(sf::RenderTarget& target)
    {
        if (m_timeline.isEmpty())
        {
            return;
        }

        int curIdx = m_timeline.getCurrentIdx();
        int nextIdx = m_timeline.getNextIdx();
        float alpha = m_timeline.getAlpha();

        const auto& curSnap = m_timeline.getSnapshot(curIdx);
        const auto& nextSnap = m_timeline.getSnapshot(nextIdx);

        // 1. GARBAGE COLLECTION: Xóa Node đã biến mất khỏi cả 2 snapshot
        for (auto it = m_nodeMap.begin(); it != m_nodeMap.end(); )
        {
            int id = it->first;
            if (curSnap.nodeStates.find(id) == curSnap.nodeStates.end() &&
                nextSnap.nodeStates.find(id) == nextSnap.nodeStates.end())
            {
                delete it->second;
                it = m_nodeMap.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // GARBAGE COLLECTION: Xóa Edge
        for (auto it = m_edgeMap.begin(); it != m_edgeMap.end(); )
        {
            auto key = it->first;
            if (curSnap.edgeStates.find(key) == curSnap.edgeStates.end() &&
                nextSnap.edgeStates.find(key) == nextSnap.edgeStates.end())
            {
                delete it->second;
                it = m_edgeMap.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // 2. VẼ VÀ NỘI SUY EDGE (Vẽ cạnh trước để nằm dưới Node)
        for (auto const& [key, nextEdgeState] : nextSnap.edgeStates)
        {
            if (m_edgeMap.find(key) == m_edgeMap.end())
            {
                // Bảo đảm 2 Node đã tồn tại trên map trước khi nối dây
                if (m_nodeMap.count(key.first) && m_nodeMap.count(key.second))
                {
                    m_edgeMap[key] = new GUI::EdgeUI(m_nodeMap[key.first], m_nodeMap[key.second], 10.0f);
                }
            }

            if (m_edgeMap.count(key))
            {
                GUI::EdgeUI* ui = m_edgeMap[key];

                Utils::Core::EdgeState curEdgeState = nextEdgeState; // Fallback
                if (curSnap.edgeStates.count(key))
                {
                    curEdgeState = curSnap.edgeStates.at(key);
                }

                Utils::Core::EdgeState interpState = Utils::Math::Interpolator::interpolateEdge(curEdgeState, nextEdgeState, alpha);

                // Áp dụng thông số nội suy vào UI
                ui->setColorFilling(interpState.fillProgress, interpState.baseFillColor, interpState.fillColor, interpState.fillFromStart);
                ui->setFocusState(interpState.isFocused);

                if (interpState.isPulsing)
                {
                    ui->setPulse(interpState.pulseProgress, interpState.pulseColor);
                }
                else
                {
                    ui->stopPulse();
                }

                ui->draw(target);
            }
        }

        // 3. VẼ VÀ NỘI SUY NODE
        for (auto const& [id, nextNodeState] : nextSnap.nodeStates)
        {
            if (m_nodeMap.find(id) == m_nodeMap.end())
            {
                m_nodeMap[id] = new GUI::NodeUI(m_font);
                m_nodeMap[id]->setExactPosition(nextNodeState.position.x, nextNodeState.position.y);
            }

            GUI::NodeUI* ui = m_nodeMap[id];

            Utils::Core::NodeState curNodeState = nextNodeState; // Fallback nếu node mới xuất hiện
            if (curSnap.nodeStates.count(id))
            {
                curNodeState = curSnap.nodeStates.at(id);
            }

            // Gọi hàm Interpolator để lấy trạng thái trung gian
            Utils::Core::NodeState interpState = Utils::Math::Interpolator::interpolateNode(curNodeState, nextNodeState, alpha, nextSnap.transitionFromPrevious);

            // Gán Target cho lò xo
            ui->setTargetPosition(interpState.position.x, interpState.position.y);
            ui->setTargetScale(interpState.scale);
            ui->setExactColor(interpState.fillColor, interpState.textColor, interpState.outlineColor);
            ui->setValue(interpState.value);

            ui->draw(target);
        }
    }

    void Heap::setMousePosition(sf::Vector2f pos)
    {
        for (auto& pair : m_edgeMap)
        {
            pair.second->setMousePosition(pos);
        }
    }
}
