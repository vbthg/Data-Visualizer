#pragma once
#include <vector>
#include <string>
#include "Command.h"
#include "TimelineManager.h"

namespace DS
{
    class DataStructure
    {
    protected:
        Core::TimelineManager* m_timeline; // Lưu con trỏ quản lý lịch sử

    public:
        DataStructure() : m_timeline(nullptr) {}
        virtual ~DataStructure() = default;

        // Hàm Setter để VisualizerState truyền Timeline vào
        void setTimelineManager(Core::TimelineManager* tm)
        {
            m_timeline = tm;
            if(m_timeline) m_timeline->clear();
        }

        virtual void updateNodePosition(int id, sf::Vector2f newPos) { };

        // BẮT BUỘC: Lớp con phải trả về danh sách các lệnh (Insert, Delete,...) để UI tạo nút bấm
        virtual std::vector<Command> getCommands() = 0;

        // BẮT BUỘC: Lớp con phải có tên để hiển thị lên UI
        virtual std::string getName() const = 0;
    };
}
