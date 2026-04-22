#pragma once

#include "DataStructure.h"
#include "TimelineManager.h"
#include "Command.h"
#include "Theme.h"
#include <vector>
#include <string>
#include <memory>

namespace DS
{

    class Heap : public DataStructure
    {
    private:
        std::vector<int> m_data;           // Mảng chứa dữ liệu Heap
//        Core::TimelineManager* m_timeline; // Con trỏ để đẩy Snapshots
        bool m_isMaxHeap;                  // true: Max Heap, false: Min Heap

        // Cấu hình Layout (Tọa độ thế giới 1920x1080)
        const float START_X = 960.f;       // Giữa màn hình
        const float START_Y = 180.f;       // Cách đỉnh màn hình 180px
        const float H_GAP = 130.f;         // Khoảng cách ngang gốc
        const float V_GAP = 140.f;         // Khoảng cách dọc giữa các tầng

    public:
        Heap(bool isMax = true);
        ~Heap() = default;

        // --- Chức năng chính theo yêu cầu đồ án ---
        void insert(int value);
        void extractRoot();                      // Extract Max (MaxHeap) hoặc Min (MinHeap)
        void updateNodeValue(int idx, int val);  // Update giá trị tại index bất kỳ
        void toggleHeapType();                   // Chuyển đổi Max <-> Min Heap
        void buildHeap();                        // Xây dựng lại Heap từ mảng hiện tại

        // --- Hệ thống UI Command ---
        std::vector<DS::Command> getCommands() override;

        std::string getName() const override { return "Binary Heap"; }

    private:
//        std::vector<int> m_data;
        std::vector<int> m_nodeIds; // Lưu trữ ID cố định cho từng phần tử
        int m_nextUniqueId = 0;     // Biến đếm để tạo ID không trùng lặp


        // --- Helper Thuật toán ---
        void siftUp(int index);
        void siftDown(int index);
        bool compare(int childVal, int parentVal); // Hàm so sánh dựa trên loại Heap hiện tại

        // --- Helper Visualizing (Ghi hình) ---

        // Tạo một Snapshot cơ bản dựa trên m_data hiện tại (đã nạp sẵn vị trí và Edge)
        std::shared_ptr<Core::ISnapshot> createBaseSnapshot(std::string op, std::string msg, const std::map<int, sf::Color>& overrides = { });

        // Ghi hình bước so sánh (Highlight 2 Node)
        void recordCompare(int idx1, int idx2, std::string msg);

        // Ghi hình bước Swap (Cấu hình Orbital Transition cho 2 Node)
        void recordSwap(int idx1, int idx2, std::string msg);

        // Ghi hình trạng thái cuối cùng (Highlight màu Success)
        void recordSuccess(std::string op, std::string msg);

        // --- Helper Toán học ---
        sf::Vector2f getPos(int index); // Tính tọa độ (x, y) dựa trên chỉ số mảng
    };

}
