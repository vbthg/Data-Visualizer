#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "ISnapshot.h"
#include "Interpolator.h"
#include "Smoothing.h"
#include "PseudoCodeBox.h"
#include "PseudoCodeRegistry.h"
#include <iostream>

namespace Core
{
    class TimelineManager
    {
    private:
        std::vector<std::shared_ptr<ISnapshot>> m_snapshots;
        float m_cursor;      // Vị trí hiện tại (có thể là số thực để nội suy)
        float m_targetCursor;
        float m_playbackSpeed;
        bool m_isPlaying;
        bool m_isReviewing; // Bật khi người dùng tương tác với HistoryBoard
        std::string m_lastLoadedMacroKey = "";
        GUI::PseudoCodeBox* codeBox = nullptr;

    public:
        TimelineManager() : m_cursor(0.0f), m_targetCursor(0.f), m_playbackSpeed(1.f), m_isPlaying(false), m_isReviewing(false) {}

        // --- QUẢN LÝ DỮ LIỆU (History Logic) ---

        void syncCodeBox(GUI::PseudoCodeBox* box) { codeBox = box; }

        void setReviewMode(bool reviewing) { m_isReviewing = reviewing; }

        void onNewMacroStarted()
        {
            if(m_snapshots.empty()) return;

            // Đặt đích đến là snapshot cuối cùng của macro cũ
            m_targetCursor = (float)m_snapshots.size() - 1.0f;
            m_isPlaying = true;
        }

        void addSnapshot(std::shared_ptr<ISnapshot> s)
        {
            if(m_isReviewing)
            {
                // Nếu đang xem lại mà có hành động mới -> Cắt bỏ tương lai
                int currentIdx = (int)m_cursor;
                if(currentIdx < (int)m_snapshots.size() - 1)
                {
                    m_snapshots.erase(m_snapshots.begin() + currentIdx + 1, m_snapshots.end());
                }
                m_isReviewing = false; // Thoát chế độ xem lại để quay về chế độ Real-time
            }

            m_snapshots.push_back(s);
            m_isPlaying = true;

//            std::cout << "[SNAPSHOTS SIZE]: " << m_snapshots.size() << "\n";
        }

        // --- ĐIỀU KHIỂN THỜI GIAN (Playback Logic) ---

        void update(float dt)
        {
            if(!m_isPlaying || m_snapshots.empty()) return;

            float maxIdx = (float)m_snapshots.size() - 1.0f;

            // Trường hợp 1: Đang cần "nhảy" để đuổi kịp macro mới
            if(std::abs(m_targetCursor - m_cursor) > 0.001f)
            {
                // Sử dụng damp để đuổi kịp target một cách mượt mà
                // smoothing = 0.05f (càng nhỏ càng nhanh), minSpeed cao để dứt khoát
                m_cursor = Utils::Math::Smoothing::damp(m_cursor, m_targetCursor, 0.05f, dt, 15.0f);
            }
            // Trường hợp 2: Đang chạy playback bình thường
            else
            {
                m_cursor += dt * m_playbackSpeed;

                // Giới hạn không cho vượt quá snapshot hiện có
                if(m_cursor > maxIdx) m_cursor = maxIdx;

                // Luôn đồng bộ targetCursor theo cursor khi đang chạy thường
                m_targetCursor = m_cursor;
            }

            if(m_cursor >= maxIdx)
            {
                m_cursor = maxIdx;
                m_isPlaying = false;
            }

            syncUI(codeBox);

//            std::cout << "[M_CURSOR]: " << m_cursor << "\n";
        }

        Core::RenderFrame getCurrentFrame() const
        {
            Core::RenderFrame frame;
            if (m_snapshots.empty()) return frame;

            // Lấy 2 snapshot tại vị trí con trỏ hiện tại
            auto snapA = m_snapshots[getCurrentIdx()];
            auto snapB = m_snapshots[getNextIdx()];
            float alpha = getAlpha();

            // 1. Nội suy Nodes
            for (const auto& nodeB : snapB->nodeStates)
            {
                auto itA = std::find_if(snapA->nodeStates.begin(), snapA->nodeStates.end(),
                                       [&](const auto& n) { return n.id == nodeB.id; });

                if (itA != snapA->nodeStates.end()) {
                    frame.nodes.push_back(Utils::Math::Interpolator::interpolateNode(*itA, nodeB, alpha, nodeB.transition));
                } else {
                    // Xử lý Node mới (Fade in)
                    Core::NodeState startState = nodeB;
                    startState.scale = 0.0f;
                    startState.opacity = 0.0f;

                    float appearAlpha = std::min(1.0f, alpha / 0.3f);
                    frame.nodes.push_back(Utils::Math::Interpolator::interpolateNode(startState, nodeB, appearAlpha, nodeB.transition));
                }
            }

            // --- PHẦN 2: NODE BIẾN MẤT (Cực kỳ quan trọng) ---
            for(const auto& nodeA : snapA->nodeStates)
            {
                auto itB = std::find_if(snapB->nodeStates.begin(), snapB->nodeStates.end(),
                                       [&](const auto& n){ return n.id == nodeA.id; });

                if(itB == snapB->nodeStates.end())
                {
                    // Nếu không tìm thấy trong B -> Node này đang bị xóa
                    // Chúng ta nội suy từ trạng thái hiện tại về "hư không"
                    Core::NodeState endState = nodeA;
                    endState.scale = 0.0f;
                    endState.opacity = 0.0f;

                    // NODE BỊ XÓA: Muốn biến mất thật nhanh (biến mất hẳn khi alpha = 0.3)
                    float disappearAlpha = std::min(1.0f, alpha / 0.3f);

                    // Giữ nguyên vị trí cũ nhưng thu nhỏ và mờ dần
                    frame.nodes.push_back(Utils::Math::Interpolator::interpolateNode(nodeA, endState, disappearAlpha, Core::TransitionType::Linear));
                }
            }


            // Tương tự, bạn nên áp dụng logic này cho Edges (Cạnh) để chúng không mất đột ngột
            for(const auto& edgeA : snapA->edgeStates)
            {
                auto itB = std::find_if(snapB->edgeStates.begin(), snapB->edgeStates.end(),
                                       [&](const auto& e){ return e.startNodeId == edgeA.startNodeId && e.endNodeId == edgeA.endNodeId; });

                if(itB == snapB->edgeStates.end())
                {
                    Core::EdgeState endEdge = edgeA;
                    endEdge.opacity = 0.0f;
                    frame.edges.push_back(Utils::Math::Interpolator::interpolateEdge(edgeA, endEdge, alpha));
                }
            }

            // 2. Nội suy Edges (Cạnh)
            for (const auto& edgeB : snapB->edgeStates)
            {
                // Tìm cạnh tương ứng ở snapA (đơn giản hóa bằng cách check start/end ID)
                auto itA = std::find_if(snapA->edgeStates.begin(), snapA->edgeStates.end(),
                                       [&](const auto& e) { return e.startNodeId == edgeB.startNodeId && e.endNodeId == edgeB.endNodeId; });

                if (itA != snapA->edgeStates.end()) {
                    frame.edges.push_back(Utils::Math::Interpolator::interpolateEdge(*itA, edgeB, alpha));
                } else {
                    // Cạnh mới xuất hiện
                    Core::EdgeState startEdge = edgeB;
                    startEdge.opacity = 0.0f;
                    frame.edges.push_back(Utils::Math::Interpolator::interpolateEdge(startEdge, edgeB, alpha));
                }
            }

            return frame;
        }

        void seek(int index) { m_cursor = (float)std::clamp(index, 0, (int)m_snapshots.size() - 1); }

        // --- TRUY XUẤT (Getters) ---

        int getCurrentIdx() const { return (int)m_cursor; }
        int getNextIdx() const    { return std::min((int)m_cursor + 1, (int)m_snapshots.size() - 1); }

        // Trả về tỉ lệ % giữa 2 snapshot để vẽ Animation (0.0 -> 1.0)
        float getAlpha() const    { return m_cursor - (int)m_cursor; }

        std::shared_ptr<ISnapshot> getSnapshot(int index) const
        {
            if (index < 0 || index >= (int)m_snapshots.size()) return nullptr;
            return m_snapshots[index];
        }

        int getCount() const { return (int)m_snapshots.size(); }
        void clear() { m_snapshots.clear(); m_cursor = 0.0f; m_isPlaying = false; }

        // Playback control
        void play()  { m_isPlaying = true; }
        void pause() { m_isPlaying = false; }
        bool isPlaying() const { return m_isPlaying; }


        void syncUI(GUI::PseudoCodeBox* codeBox)
        {
            if(m_snapshots.empty()) return;

            // Lấy snapshot tại vị trí con trỏ hiện tại (phần nguyên)
            auto currentSnap = m_snapshots[getCurrentIdx()];

            // 1. Kiểm tra nếu Macro thay đổi thì mới nạp lại bộ code (Tránh nạp lại mỗi frame)
            if(currentSnap->macroKey != m_lastLoadedMacroKey)
            {
                m_lastLoadedMacroKey = currentSnap->macroKey;

                // Lấy Resource từ Registry mà chúng ta đã làm ở bước trước
                if(Resources::PseudoCodeRegistry.count(m_lastLoadedMacroKey))
                {
                    codeBox->loadCode(currentSnap->operationName,
                                     Resources::PseudoCodeRegistry.at(m_lastLoadedMacroKey));
                }
            }

            // 2. Luôn cập nhật dòng highlighter và giá trị biến
            codeBox->updateStep(currentSnap->pseudoCodeLine, currentSnap->variableStates);
        }
    };



}


//#pragma once
//#include <vector>
//#include <algorithm>
//#include "Snapshot.h"
//
//namespace Core
//{
//    class TimelineManager
//    {
//    private:
//        std::vector<Core::Snapshot> m_snapshots;
//        float m_cursor;         // Con trỏ thời gian (VD: 1.5 là ở giữa bước 1 và 2)
//        float m_playbackSpeed;
//        bool m_isPlaying;
//
//    public:
//        TimelineManager()
//        {
//            m_cursor = 0.0f;
//            m_playbackSpeed = 3.0f;
//            m_isPlaying = false;
//        }
//
//        void addSnapshot(const Snapshot& s)
//        {
//            m_snapshots.push_back(s);
//        }
//
//        // CỰC KỲ QUAN TRỌNG: Cắt bỏ tương lai cũ khi người dùng rẽ nhánh lịch sử
//        void truncateAtCurrent()
//        {
//            if(m_snapshots.empty()) return;
//
//            int currentIndex = (int)m_cursor;
//            if(currentIndex < m_snapshots.size() - 1)
//            {
//                m_snapshots.erase(m_snapshots.begin() + currentIndex + 1, m_snapshots.end());
//            }
//        }
//
//        void update(float dt)
//        {
//            if(!m_isPlaying || m_snapshots.empty()) return;
//
//            m_cursor += dt * m_playbackSpeed;
//
//            // Khóa cursor ở snapshot cuối cùng
//            if(m_cursor >= (float)m_snapshots.size() - 1.0f)
//            {
//                m_cursor = (float)m_snapshots.size() - 1.0f;
//                m_isPlaying = false;
//            }
//        }
//
//        bool isEmpty() const
//        {
//            return m_snapshots.empty();
//        }
//
//        int getSnapshotsCount() const
//        {
//            return m_snapshots.size();
//        }
//
//        // Lấy phần thập phân để tính nội suy (VD: 1.7 -> trả về 0.7)
//        float getAlpha() const
//        {
//            return m_cursor - (int)m_cursor;
//        }
//
//        int getCurrentIdx() const
//        {
//            return (int)m_cursor;
//        }
//
//        int getNextIdx() const
//        {
//            return std::min((int)m_cursor + 1, (int)m_snapshots.size() - 1);
//        }
//
//        const Snapshot& getSnapshot(int index) const
//        {
//            return m_snapshots[index];
//        }
//
//        // Dùng cho tính năng Tua tiến/Tua lùi (Time Travel / Scrubbing)
//        void seek(float targetStep)
//        {
//            if(m_snapshots.empty()) return;
//            m_cursor = std::max(0.0f, std::min(targetStep, (float)m_snapshots.size() - 1.0f));
//        }
//
//        void play() { m_isPlaying = true; }
//        void pause() { m_isPlaying = false; }
//
//        bool isAtEnd() const
//        {
//            return m_snapshots.empty() || m_cursor >= (float)m_snapshots.size() - 1.0f;
//        }
//
//        void clear()
//        {
//            m_snapshots.clear();
//            m_cursor = 0.0f;
//            m_isPlaying = false;
//        }
//    };
//}
