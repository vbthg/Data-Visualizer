#pragma once
#include <string>
#include <vector>
#include <SFML/System/String.hpp>

namespace Core
{
    // Đại diện cho một bước nhỏ trong thuật toán (ví dụ: Compare, Swap)
    struct HistorySubStep
    {
        std::string description = "";
        int snapshotIndex = 0; // Để nhảy (seek) trong TimelineManager
    };

    // Đại diện cho một thao tác lớn của người dùng (ví dụ: Insert, Delete)
    struct HistoryOperation
    {
        std::string name = "";
        sf::String iconCode = ""; // Lưu mã Unicode của Icon
        std::vector<HistorySubStep> subSteps = {};

        int finalSnapshotIndex = 0; // Snapshot khi thao tác này kết thúc hoàn toàn
        bool isExpanded = true; // Trạng thái UI (Mở/Đóng folder)
        float expansionProgress = 1.0f; // 0.0: Đóng hoàn toàn, 1.0: Mở hoàn toàn
//        bool isSuccess = true;  // Để hiển thị trạng thái hoàn thành
    };
}
