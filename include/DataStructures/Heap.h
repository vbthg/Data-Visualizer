#pragma once
#include "DataStructure.h"
#include <vector>
#include <string>

namespace DS
{
    // Cấu trúc bắt buộc để giữ ID cố định cho mỗi giá trị (phục vụ Animation)
    struct HeapNode
    {
        int value;
        int id;
    };

    class Heap : public DataStructure
    {
    private:
        std::vector<HeapNode> m_data;
        bool m_isMaxHeap;
        int m_nextId;

        // Cấu hình hiển thị Apple-style
        const float LEVEL_HEIGHT = 100.0f;
        const float BASE_X_GAP = 450.0f; // Khoảng cách giãn ngang ở tầng cao nhất

        // Helpers
        int getParent(int i) { return (i - 1) / 2; }
        int getLeft(int i) { return 2 * i + 1; }
        int getRight(int i) { return 2 * i + 2; }

        // Tính toán tọa độ để không đè nhau (Responsive theo tầng)
        sf::Vector2f calculateNodePos(int index);

        // Hàm chụp Snapshot chi tiết theo yêu cầu
        void saveState(std::string title, std::string subtitle, GUI::Scenario scenario,
                       std::string macroKey, int line,
                       int activeIdx = -1, int targetIdx = -1);

        // Các hàm phụ trợ thuật toán
        void siftUp(int index, std::string macroTitle);
        void siftDown(int index, std::string macroTitle);

    public:
        Heap();
        virtual ~Heap() = default;

        // Implement DataStructure interface
        bool loadFromFile(const std::string& path) override;
        std::string getName() const override { return m_isMaxHeap ? "Binary Max-Heap" : "Binary Min-Heap"; }
        std::vector<Command> getCommands() override;

        // Các hành động chính
        void insert(int value);
        void extractRoot();
        void updateNodeValue(int index, int newValue);
        void toggleHeapType();
    };
}
