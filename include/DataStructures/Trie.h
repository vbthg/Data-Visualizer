#pragma once
#include "DataStructure.h"
#include <map>
#include <string>
#include <vector>

namespace DS
{
    class Trie : public DataStructure
    {
    private:
        // Cấu trúc một nút trong cây tiền tố
        struct Node
        {
            int id;                                 // ID duy nhất để hệ thống nội suy nhận diện
            char character;                         // Ký tự tại nút này ('\0' nếu là root)
            bool isEndOfWord;                       // Đánh dấu kết thúc một từ
            std::map<char, Node*> children;         // Danh sách các con (A-Z, 0-9,...)

            Node(char c, int nodeId) : character(c), id(nodeId), isEndOfWord(false) {}
        };

        Node* m_root;
        int m_nextId;                               // Bộ đếm để cấp phát ID không trùng lặp
        float m_startY = 150.0f;                    // Tọa độ Y bắt đầu của Root

    public:
        Trie();
        virtual ~Trie();

        // --- HỆ THỐNG COMMAND (Cho Floating Dock) ---
        // Cung cấp các nút bấm: Insert, Search, Remove, Clear
        std::vector<Command> getCommands() override;
        std::string getName() const override { return "Trie (Prefix Tree)"; }

        // --- CÁC THAO TÁC CHÍNH (LOGIC) ---
        void insert(const std::string& word);
        bool search(const std::string& word);
        void remove(const std::string& word);
        void clear();

    private:
        // --- HELPER PHỤ TRỢ ---
        bool removeRecursive(Node* curr, const std::string& word, int index, bool& deleted);
        void clearRecursive(Node* node);

        // --- HỆ THỐNG SNAPSHOT & VISUALIZATION ---

        /**
         * @brief Chụp và đóng gói toàn bộ trạng thái (Visual & Logic) vào một Snapshot.
         * * Snapshot này sẽ được TimelineManager lưu trữ để phục vụ việc nội suy animation,
         * hiển thị mã giả tương ứng và cập nhật giá trị các biến số lên UI.
         *
         * @param highlightNodeId ID của Node đang được xử lý (để đổi màu/phóng to).
         * @param message         Lời nhắn giải thích hành động (hiện trên Notch/Log).
         * @param macroKey        Từ khóa để nạp bộ mã giả (ví dụ: "trie_insert", "trie_search").
         * @param opName          Tên hành động lớn hiển thị trên tiêu đề bảng Code (ví dụ: "TRIE: INSERT").
         * @param lineIdx         Số thứ tự dòng code cần được tô sáng trong bảng PseudoCodeBox.
         * @param vars            Danh sách các cặp {Tên biến, Giá trị} để hiển thị trong mục Local Variables.
         * @param isWarning       Cờ đánh dấu trạng thái cảnh báo (đổi màu Node highlight sang đỏ/cam).
         */
                // Thêm vào phần private của class Trie trong Trie.h
        void createSnapshot(int highlightNodeId,
                                const std::string& message,
                                const std::string& macroKey,    // Thêm cái này
                                const std::string& opName,      // Thêm cái này
                                int lineIdx,                    // Thêm cái này
                                std::vector<std::pair<std::string, std::string>> vars = {}, // Thêm cái này
                                bool isWarning = false);

        /**
         * @brief Tính toán vị trí các node để chúng không đè lên nhau
         * @note Đây là phần khó nhất của Trie vì số lượng nhánh không cố định
         */
        void computeLayout(Node* node, float x, float y, float horizontalRange,
                           std::vector<Core::NodeState>& nodes,
                           std::vector<Core::EdgeState>& edges);
    };
}

/**
 * --- TỪ VỰNG TIẾNG ANH CHUYÊN NGÀNH (English Terminology) ---
 * 1. Prefix: Tiền tố (ví dụ "app" là tiền tố của "apple").
 * 2. Traversal: Quá trình duyệt qua các nút của cây.
 * 3. End-of-word marker: Cờ đánh dấu đã hoàn thành một từ hợp lệ.
 * 4. Backtracking: Quá trình quay lui (sử dụng trong hàm remove để xóa các node rác).
 * 5. Branching factor: Hệ số phân nhánh (Trie có hệ số này rất cao).
 */
