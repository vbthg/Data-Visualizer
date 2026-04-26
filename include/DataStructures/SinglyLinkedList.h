#pragma once
#include "DataStructure.h"
#include <vector>
#include <string>

namespace DS
{
    // Cấu trúc dữ liệu logic cho một Node trong Linked List
    struct Node
    {
        int id;             // ID định danh duy nhất (không đổi để Interpolator nhận diện)
        int value;          // Giá trị chứa trong Node
        sf::Vector2f pos;   // Tọa độ mục tiêu của Node trên màn hình
    };

    class SinglyLinkedList : public DataStructure
    {
    private:
        std::vector<Node> m_nodes;  // Danh sách các Node hiện có (logic)
        int m_headId;               // ID của Node đang là Head
        int m_nextNodeId;           // Counter để cấp phát ID không trùng lặp

        // Hằng số hiển thị
        const float NODE_DISTANCE = 200.0f; // Khoảng cách giữa các toa tàu
        const float Y_LEVEL = 540.0f;       // Độ cao mặc định của danh sách (giữa màn hình)
        const int GHOST_NULL_ID = -999;     // ID đặc biệt cho Node NULL

    public:
        SinglyLinkedList();
        virtual ~SinglyLinkedList();

        // --- CÁC THAO TÁC CƠ BẢN (MINH HỌA THUẬT TOÁN) ---

        // Thêm vào đầu danh sách (O(1))
        void pushFront(int value);

        // Thêm vào cuối danh sách (O(n))
        void pushBack(int value);

        // Chèn vào vị trí bất kỳ (O(n))
        void insert(int index, int value);

        // Xóa Node theo giá trị hoặc vị trí
        void remove(int index);

        // Cập nhật giá trị tại vị trí index
        void updateValue(int index, int newValue);

        // Xóa toàn bộ danh sách
        void clear();

        // --- HÀM HỖ TRỢ VISUALIZER ---

        // Chụp lại trạng thái hiện tại để đẩy vào Timeline
        // Scenario: Trạng thái của Notch (Success/Processing/...)
        // highlightNodeId: ID của Node cần đổi màu vàng để nhấn mạnh
        // Hàm createSnapshot nâng cấp để nhận thêm biến cục bộ
        void createSnapshot(GUI::Scenario scenario,
                            const std::string& message,
                            int lineIdx,
                            int highlightNodeId = -1,
                            std::vector<std::pair<std::string, std::string>> vars = {},
                            float customNullX = -1);

        // Cập nhật lại tọa độ x cho tất cả Node để dàn hàng ngang mượt mà
        void recomputePositions();

        // Implement các hàm ảo từ DataStructure
        virtual std::vector<Command> getCommands() override;
        virtual std::string getName() const override { return "SINGLY LINKED LIST"; }
        virtual void updateNodePosition(int id, sf::Vector2f newPos) override;

    private:
        // Tìm vị trí của Node trong vector dựa trên ID
        int findNodeIdxById(int id);

        // Tìm vị trí của Node dựa trên index (stt trong danh sách)
        int findNodeIdByIndex(int index);
    };
}
