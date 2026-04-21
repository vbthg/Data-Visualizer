#pragma once
#include <vector>
#include <map>
#include <string>
#include "AnimationMetadata.h"
#include "ISnapshot.h"

namespace Core
{
    class Snapshot : public Core::ISnapshot
    {
    public:
        // ID của thao tác lớn (Macro-step), ví dụ: 1 cho Insert(10), 2 cho ExtractMax()
        // Các Snapshot có cùng macroStepID sẽ được gom vào 1 mục thả xuống trong History Panel
        int macroStepID = -1;

        std::map<int, NodeState> nodeStates;

        // Key có thể là std::pair<int, int> (StartNodeID, EndNodeID)
        std::map<std::pair<int, int>, EdgeState> edgeStates;

        TransitionType transitionFromPrevious = TransitionType::Linear;
        std::string logMessage;

        Snapshot() = default;
    };
}
