#pragma once
#include "DataStructure.h"
#include "GUI/NodeUI.h"
#include "GUI/EdgeUI.h"
#include "TimelineManager.h" // Thay thế AnimationManager
#include "Snapshot.h"
#include <map>
#include <vector>
#include <string>

namespace DS
{
    // Cấu trúc lõi: Mỗi giá trị đi kèm 1 ID định danh vĩnh viễn
    struct HeapElement
    {
        int value;
        int id;
    };

    class Heap : public DataStructure
    {
    private:
        std::vector<HeapElement> m_data; // Dữ liệu logic (Bây giờ chứa cả ID)
        int m_idCounter = 0;             // Bộ đếm sinh ID độc nhất
        int m_currentMacroStep = 0;

        // Hệ thống Mapping ID -> UI Thực thể (Quản lý rác tự động)
        std::map<int, GUI::NodeUI*> m_nodeMap;
        std::map<std::pair<int, int>, GUI::EdgeUI*> m_edgeMap;

        Utils::Core::TimelineManager m_timeline;
        sf::Font* m_font;
        GUI::DynamicIsland* m_island; // Để truyền logMessage trực tiếp nếu cần

        // --- HÀM HỖ TRỢ KIẾN TRÚC MỚI ---

        // Máy ảnh: Chụp lại trạng thái của m_data hiện tại ném vào Timeline
        void captureSnapshot(const std::string& logMessage, Utils::Core::TransitionType transition, int macroStepID = -1);

        // Toán học: Tính toán tọa độ lý thuyết của Node thứ 'index' trên cây
        sf::Vector2f getTheoreticalPosition(int index);

        // --- CÁC HÀM THUẬT TOÁN (Chỉ còn logic thuần túy) ---
        void heapifyUp(int index, int macroStepID);
        void heapifyDown(int index, int macroStepID);

    public:
        Heap();
        ~Heap() override;

        void insert(int value);
        void extractMax();
        void clear();

        std::vector<DS::Command> getCommands() override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;
        std::string getName() const override;

        void bindDynamicIsland(GUI::DynamicIsland* island) override;
        void setMousePosition(sf::Vector2f pos) override;
    };
}
