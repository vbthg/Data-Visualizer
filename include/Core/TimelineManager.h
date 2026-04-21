#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "ISnapshot.h"
#include "Interpolator.h"

namespace Core
{
    class TimelineManager
    {
    private:
        std::vector<std::shared_ptr<ISnapshot>> m_snapshots;
        float m_cursor;      // Vị trí hiện tại (có thể là số thực để nội suy)
        float m_playbackSpeed;
        bool m_isPlaying;

    public:
        TimelineManager() : m_cursor(0.0f), m_playbackSpeed(0.6f), m_isPlaying(false) {}

        // --- QUẢN LÝ DỮ LIỆU (History Logic) ---

        void addSnapshot(std::shared_ptr<ISnapshot> s)
        {
            // Nếu đang ở giữa mà thêm cái mới -> Xóa "tương lai" (Branching history)
            int currentIdx = (int)m_cursor;
            if (currentIdx < (int)m_snapshots.size() - 1)
            {
                m_snapshots.erase(m_snapshots.begin() + currentIdx + 1, m_snapshots.end());
            }

            m_snapshots.push_back(s);
            // Thường khi thêm mới, ta muốn cursor nhảy đến cuối luôn
            m_cursor = (float)m_snapshots.size() - 1.0f;
        }

        // --- ĐIỀU KHIỂN THỜI GIAN (Playback Logic) ---

        void update(float dt)
        {
            if (!m_isPlaying || m_snapshots.empty()) return;

            m_cursor += dt * m_playbackSpeed;

            if (m_cursor >= (float)m_snapshots.size() - 1.0f)
            {
                m_cursor = (float)m_snapshots.size() - 1.0f;
                m_isPlaying = false;
            }
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
                    frame.nodes.push_back(Utils::Math::Interpolator::interpolateNode(startState, nodeB, alpha, nodeB.transition));
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
