#include "Heap.h"
#include "TreeLayoutUtils.h"
#include "Theme.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

namespace DS
{
    namespace Theme = Utils::Graphics::Theme;

    Heap::Heap( bool isMax)
        : m_isMaxHeap(isMax)
    {
        // Khởi tạo hạt giống ngẫu nhiên
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

//        if (m_timeline) {
//            m_timeline->clear();
//        }
    }

    // --- MATHEMATICAL HELPERS ---

    sf::Vector2f Heap::getPos(int i)
    {
        if (i < 0) return { 0, 0 };

        int depth = static_cast<int>(std::log2(i + 1));
        int posInLevel = i - (static_cast<int>(std::pow(2, depth)) - 1);

        // Tính toán xFactor để căn giữa cây
        float xFactor = static_cast<float>(2 * posInLevel - (std::pow(2, depth) - 1));

        // Gọi Utils để lấy tọa độ thực tế
        return Utils::TreeLayoutUtils::getBinaryTreeNodePos(
            depth,
            xFactor,
            {START_X, START_Y},
            H_GAP,
            V_GAP
        );
    }

    bool Heap::compare(int childVal, int parentVal)
    {
        // Logic so sánh dựa trên loại Heap (Max/Min)
        return m_isMaxHeap ? (childVal > parentVal) : (childVal < parentVal);
    }

    // --- VISUALIZING HELPERS ---

    std::shared_ptr<Core::ISnapshot> Heap::createBaseSnapshot(std::string op, std::string msg)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        snap->operationName = op; // Title (English)
        snap->logMessage = msg;    // Subtitle (English)
        snap->scenario = GUI::Scenario::Processing;

        // Ghi lại trạng thái Node
        for (int i = 0; i < (int)m_data.size(); ++i)
        {
            Core::NodeState ns;
            ns.id = i;
            ns.value = std::to_string(m_data[i]);
            ns.position = getPos(i);
            ns.fillColor = Theme::Color::NodeDefault;
            ns.transition = Core::TransitionType::Linear;
            snap->nodeStates.push_back(ns);
        }

        // Ghi lại trạng thái Cạnh
        for (int i = 1; i < (int)m_data.size(); ++i)
        {
            Core::EdgeState es;
            es.startNodeId = (i - 1) / 2;
            es.endNodeId = i;
            snap->edgeStates.push_back(es);
        }

        return snap;
    }

    void Heap::recordCompare(int idx1, int idx2, std::string msg)
    {
        auto snap = createBaseSnapshot("Comparing", msg);
        // Highlight các node đang so sánh bằng màu vàng
        if (idx1 < (int)snap->nodeStates.size()) snap->nodeStates[idx1].fillColor = sf::Color::Yellow;
        if (idx2 < (int)snap->nodeStates.size()) snap->nodeStates[idx2].fillColor = sf::Color::Yellow;

        m_timeline->addSnapshot(snap);
    }

    void Heap::recordSwap(int idx1, int idx2, std::string msg)
    {
        auto snap = createBaseSnapshot("Swapping", msg);

        // Sử dụng Orbital Transition để 2 node bay vòng cung khi hoán đổi
        if (idx1 < (int)snap->nodeStates.size()) snap->nodeStates[idx1].transition = Core::TransitionType::Orbital;
        if (idx2 < (int)snap->nodeStates.size()) snap->nodeStates[idx2].transition = Core::TransitionType::Orbital;

        m_timeline->addSnapshot(snap);
    }

    void Heap::recordSuccess(std::string op, std::string msg)
    {
        auto snap = createBaseSnapshot(op, msg);
        snap->scenario = GUI::Scenario::Success;

        // Đổi màu xanh lá cho toàn bộ cây khi hoàn thành thao tác
        for (auto& node : snap->nodeStates) node.fillColor = sf::Color(150, 255, 150);

        m_timeline->addSnapshot(snap);
    }

    // --- CORE ALGORITHMS ---

    void Heap::insert(int value)
    {
        m_data.push_back(value);
        recordSuccess("Insert", "New node " + std::to_string(value) + " added to the end.");

        siftUp((int)m_data.size() - 1);
    }

    void Heap::siftUp(int index)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;
            recordCompare(index, parent, "Comparing child with parent.");

            if (compare(m_data[index], m_data[parent]))
            {
                std::swap(m_data[index], m_data[parent]);
                recordSwap(index, parent, "Heap property violated! Swapping nodes.");
                index = parent;
            }
            else break;
        }
        recordSuccess("Sift Up Done", "The node has reached its correct position.");
    }

    void Heap::extractRoot()
    {
        if (m_data.empty()) return;

        recordCompare(0, 0, "Removing the root element.");

        if (m_data.size() > 1)
        {
            // Đưa node cuối lên đầu để bắt đầu vun đống xuống
            std::swap(m_data[0], m_data.back());
            recordSwap(0, (int)m_data.size() - 1, "Moving the last leaf to the root.");
            m_data.pop_back();

            siftDown(0);
        }
        else
        {
            m_data.pop_back();
            recordSuccess("Extract Done", "The last remaining node was removed.");
        }
    }

    void Heap::siftDown(int index)
    {
        int size = (int)m_data.size();
        while (true)
        {
            int target = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < size && compare(m_data[left], m_data[target])) target = left;
            if (right < size && compare(m_data[right], m_data[target])) target = right;

            if (target != index)
            {
                recordCompare(index, target, "Comparing parent with children.");
                std::swap(m_data[index], m_data[target]);
                recordSwap(index, target, "Swapping parent with the larger/smaller child.");
                index = target;
            }
            else break;
        }
        recordSuccess("Sift Down Done", "Heapification complete.");
    }

    void Heap::updateNodeValue(int idx, int val)
    {
        if (idx < 0 || idx >= (int)m_data.size()) return;

        int oldVal = m_data[idx];
        m_data[idx] = val;

        auto snap = createBaseSnapshot("Update Node", "Changed value at index " + std::to_string(idx) + " to " + std::to_string(val));
        snap->nodeStates[idx].fillColor = sf::Color::Cyan; // Highlight node vừa cập nhật
        m_timeline->addSnapshot(snap);

        // Tự động quyết định hướng vun đống dựa trên giá trị mới
        if (compare(val, oldVal)) siftUp(idx);
        else siftDown(idx);
    }

    void Heap::toggleHeapType()
    {
        m_isMaxHeap = !m_isMaxHeap;
        buildHeap();
    }

    void Heap::buildHeap()
    {
        std::string type = m_isMaxHeap ? "Max Heap" : "Min Heap";
        recordSuccess("Rebuilding", "Converting structure to " + type);

        // Xây dựng lại từ các node trung gian ngược lên gốc
        for (int i = (int)m_data.size() / 2 - 1; i >= 0; --i)
        {
            siftDown(i);
        }
    }

    std::vector<DS::Command> Heap::getCommands()
    {
        return {
            { L"\uE3D6", DS::InputType::Integer, [this](DS::InputArgs args) { this->insert(args.iVal1); }}, // insert
            { L"\uE616", DS::InputType::None, [this](DS::InputArgs args) { this->extractRoot(); }},    // extract root
            { L"\uE3AA", DS::InputType::TwoIntegers, [this](DS::InputArgs args) { this->updateNodeValue(args.iVal1, args.iVal2); }}, // update
            { L"\uE098", DS::InputType::None, [this](DS::InputArgs args) { this->toggleHeapType(); }}  // toggle max/min
        };
    }

}
