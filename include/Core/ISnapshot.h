// ISnapshot.h
#pragma once

#include "AnimationMetadata.h"
#include "NotchEnums.h"
#include "vector"
#include "string"

namespace Core
{
    struct NotchContext
    {
        GUI::Scenario scenario;    // Để biết là Processing, Success, hay Error...
        std::string title;         // Chính là operationName (VD: "AVL Rotation")
        std::string subtitle;      // Chính là logMessage (VD: "Left-Left case detected")
        std::string iconCode;      // Mã icon (VD: "\xef\x80\x8c" cho dấu tick)

        int step = 0;              // Bước thứ i trong Macro
        int total = 0;             // Tổng bước trong Macro

        // Nếu em muốn mỗi bước có màu riêng (hiếm nhưng có thể),
        // có thể thêm sf::Color customColor ở đây.
    };

    struct CodeContext
    {
        std::string macroKey;
        int pseudoCodeLine = 0;
        std::vector<std::pair<std::string, std::string>> variableStates;
    };

    class ISnapshot
    {
    public:
        virtual ~ISnapshot() = default;

        std::vector<NodeState> nodeStates;
        std::vector<EdgeState> edgeStates;

        NotchContext notchData;
        CodeContext codeData;

        // Các metadata khác
        int snapshotIndex;
        int macroStepID;
    };
}
