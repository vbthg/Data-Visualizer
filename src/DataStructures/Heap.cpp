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
        if(m_data.empty() || i < 0) return {START_X, START_Y};

        int depth = static_cast<int>(std::log2(i + 1));
        int maxDepth = static_cast<int>(std::log2(m_data.size()));

        // 1. TÍNH TOÁN V_GAP ĐỘNG (Ý tưởng của bạn)
        // Mỗi khi cây thêm 1 tầng, chúng ta kéo dãn khoảng cách dọc của TẤT CẢ các tầng
        // Ví dụ: Mỗi tầng tăng thêm 20px cho mỗi cấp độ sâu của cây
        float dynamicVGap = V_GAP + (maxDepth * 20.0f);

        // 2. TÍNH TOÁN H_GAP ĐỘNG (Để tránh đè nhau)
        // H_GAP cơ sở cũng nên thu nhỏ lại một chút khi cây quá sâu
        float baseHGap = H_GAP;
        if(maxDepth > 3)
        {
            baseHGap *= std::pow(0.8f, maxDepth - 3);
        }

        // 3. TÍNH TOÁN X (Đảm bảo trung trực)
        // Sử dụng cơ chế: x = START_X + (offset_tương_đối)
        float x = START_X;
        int tempIdx = i;

        // Duyệt ngược từ node i lên Root để xác định vị trí X
        // Mỗi bước lên cha, chúng ta cộng/trừ một khoảng cách lũy thừa của 2
        int currentD = depth;
        while(tempIdx > 0)
        {
            // Khoảng cách ngang giữa cha và con ở tầng currentD-1
            float levelOffset = baseHGap * std::pow(2.0f, maxDepth - currentD);

            if(tempIdx % 2 == 1) x -= levelOffset; // Nếu là con trái thì node ở bên trái cha
            else x += levelOffset;                 // Nếu là con phải thì node ở bên phải cha

            tempIdx = (tempIdx - 1) / 2;
            currentD--;
        }

        // 4. TÍNH TOÁN Y
        // Sử dụng V_GAP đã được dãn ra cho toàn bộ cây
        float y = START_Y + depth * dynamicVGap;

        return {x, y};
    }

    bool Heap::compare(int childVal, int parentVal)
    {
        // Logic so sánh dựa trên loại Heap (Max/Min)
        return m_isMaxHeap ? (childVal > parentVal) : (childVal < parentVal);
    }

    // --- VISUALIZING HELPERS ---

    // --- TRONG HÀM CREATE BASE SNAPSHOT ---
    std::shared_ptr<Core::ISnapshot> Heap::createBaseSnapshot(std::string op, std::string msg, const std::map<int, sf::Color>& overrides)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        // ... (giữ nguyên logic title/msg)

        for(int i = 0; i < (int)m_data.size(); ++i)
        {
            Core::NodeState ns;
            // QUAN TRỌNG: ID phải lấy từ mảng m_nodeIds, không phải lấy chỉ số i
            ns.id = m_nodeIds[i];
            ns.value = std::to_string(m_data[i]);
            ns.position = getPos(i);

            // Logic override màu sắc:
            // Lưu ý: overrides.find(i) ở đây 'i' vẫn là index, điều này ổn
            // nếu bạn truyền index từ hàm recordSwap vào.
            auto it = overrides.find(i);
            if(it != overrides.end())
            {
                ns.fillColor = it->second;
            }
            else
            {
                ns.fillColor = sf::Color(230, 183, 167);
            }

            ns.transition = Core::TransitionType::Linear;
            snap->nodeStates.push_back(ns);
        }

        // Cập nhật cạnh (Edges) cũng phải dựa trên ID
        for(int i = 1; i < (int)m_data.size(); ++i)
        {
            Core::EdgeState es;
            es.startNodeId = m_nodeIds[(i - 1) / 2]; // ID của cha
            es.endNodeId = m_nodeIds[i];           // ID của con
            snap->edgeStates.push_back(es);
        }

        return snap;
    }

    void Heap::recordCompare(int idx1, int idx2, std::string msg)
    {
        // Chỉ định node idx1 và idx2 là màu vàng
        std::map<int, sf::Color> colors = {{idx1, sf::Color::Yellow}, {idx2, sf::Color::Yellow}};
        auto snap = createBaseSnapshot("Comparing", msg, colors);
        m_timeline->addSnapshot(snap);
    }

    void Heap::recordSwap(int idx1, int idx2, std::string msg)
    {
        // DUY TRÌ màu vàng cho idx1 và idx2 trong khi chúng hoán đổi vị trí
        std::map<int, sf::Color> colors = {{idx1, sf::Color::Yellow}, {idx2, sf::Color::Yellow}};
        auto snap = createBaseSnapshot("Swapping", msg, colors);


        // Lấy ID thực tế của 2 node để kiểm tra cạnh
        int id1 = m_nodeIds[idx1];
        int id2 = m_nodeIds[idx2];

        for(auto& edge : snap->edgeStates)
        {
            // Nếu cạnh có bất kỳ đầu mút nào là node đang di chuyển
            int cnt = (edge.startNodeId == id1 || edge.startNodeId == id2) + (edge.endNodeId == id1 || edge.endNodeId == id2);
            if(cnt == 1)
            {
                // Làm mờ hẳn đi để không gây rối mắt khi chúng cắt nhau
                edge.opacity = 0.4f;
            }
        }

        if(idx1 < (int)snap->nodeStates.size()) snap->nodeStates[idx1].transition = Core::TransitionType::Orbital;
        if(idx2 < (int)snap->nodeStates.size()) snap->nodeStates[idx2].transition = Core::TransitionType::Orbital;

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
        m_timeline->onNewMacroStarted();

        m_data.push_back(value);
        m_nodeIds.push_back(m_nextUniqueId++); // Gán một ID duy nhất cho phần tử mới

        recordSuccess("Insert", "New node " + std::to_string(value) + " added.");
        siftUp((int)m_data.size() - 1);
    }

    // --- TRONG CÁC HÀM SIFT UP / SIFT DOWN ---
    // Bạn phải swap cả m_nodeIds!
    void Heap::siftUp(int index)
    {
        while(index > 0)
        {
            int parent = (index - 1) / 2;
            recordCompare(index, parent, "Comparing child with parent.");

            if(compare(m_data[index], m_data[parent]))
            {
                // Swap dữ liệu
                std::swap(m_data[index], m_data[parent]);
                // Swap cả ID để duy trì danh tính (Identity) của Node
                std::swap(m_nodeIds[index], m_nodeIds[parent]);

                recordSwap(index, parent, "Heap property violated! Swapping nodes.");
                index = parent;
            }
            else break;
        }
        recordSuccess("Sift Up Done", "The node has reached its correct position.");
    }

    void Heap::extractRoot()
    {
        m_timeline->onNewMacroStarted();
        if(m_data.empty()) return;

        recordCompare(0, 0, "Removing the root element.");

        if(m_data.size() > 1)
        {
            std::swap(m_data[0], m_data.back());
            std::swap(m_nodeIds[0], m_nodeIds.back()); // Nhớ swap cả ID nhé!

            recordSwap(0, (int)m_data.size() - 1, "Moving the last leaf to the root.");

            // Xóa phần tử cuối
            m_data.pop_back();
            m_nodeIds.pop_back();

            // Snapshot này sẽ kích hoạt hiệu ứng "Biến mất" vì Node ID cũ không còn trong mảng nữa
            recordSuccess("Extract Done", "The node was removed and we start heapifying.");

            siftDown(0);
        }
        else
        {
            m_data.pop_back();
            m_nodeIds.pop_back();
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
                std::swap(m_nodeIds[index], m_nodeIds[target]);
                recordSwap(index, target, "Swapping parent with the larger/smaller child.");
                index = target;
            }
            else break;
        }
        recordSuccess("Sift Down Done", "Heapification complete.");
    }

    void Heap::updateNodeValue(int idx, int val)
    {
        m_timeline->onNewMacroStarted();
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
        m_timeline->onNewMacroStarted();
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
