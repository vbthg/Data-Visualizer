// ISnapshot.h
#pragma once
#include <vector>
#include <string>
#include "AnimationMetadata.h" // Chứa NodeState, EdgeState, TransitionType
#include "NotchEnums.h"

namespace Core
{
    // Để NotchManager biết nên hiện màu gì (Xanh, Đỏ, Vàng...)

    class ISnapshot
    {
    public:
        virtual ~ISnapshot() = default;

        // --- DỮ LIỆU ĐỒ HỌA (Cái để vẽ) ---
        std::vector<NodeState> nodeStates;
        std::vector<EdgeState> edgeStates;

        // --- DỮ LIỆU NGỮ CẢNH (Cái để nói) ---
        GUI::Scenario scenario; // Dùng đúng Enum của Notch
        std::string operationName; // VD: "AVL Insertion"
        std::string logMessage;    // VD: "Comparing 50 with 30..."
//        StatusType status;         // Màu sắc của Notch lúc đó

        std::string macroKey; // Lưu key như "trie_insert", "trie_search"
        int pseudoCodeLine;        // Dòng code cần highlight trong PseudoCodeBox

        // Thêm vào class ISnapshot
        std::vector<std::pair<std::string, std::string>> variableStates;

        // --- DỮ LIỆU LIÊN KẾT (Cái để quản lý) ---
        int snapshotIndex;         // Vị trí của chính nó trong Timeline
        int macroStepID;           // ID của Big Step trong HistoryBoard (để biết nó thuộc cụm nào)
    };
}
