#pragma once
#include <vector>
#include <algorithm>
#include "Snapshot.h"

namespace Utils
{
namespace Core
{
    class TimelineManager
    {
    private:
        std::vector<Snapshot> m_snapshots;
        float m_cursor;         // Con trỏ thời gian (VD: 1.5 là ở giữa bước 1 và 2)
        float m_playbackSpeed;
        bool m_isPlaying;

    public:
        TimelineManager()
        {
            m_cursor = 0.0f;
            m_playbackSpeed = 3.0f;
            m_isPlaying = false;
        }

        void addSnapshot(const Snapshot& s)
        {
            m_snapshots.push_back(s);
        }

        // CỰC KỲ QUAN TRỌNG: Cắt bỏ tương lai cũ khi người dùng rẽ nhánh lịch sử
        void truncateAtCurrent()
        {
            if(m_snapshots.empty()) return;

            int currentIndex = (int)m_cursor;
            if(currentIndex < m_snapshots.size() - 1)
            {
                m_snapshots.erase(m_snapshots.begin() + currentIndex + 1, m_snapshots.end());
            }
        }

        void update(float dt)
        {
            if(!m_isPlaying || m_snapshots.empty()) return;

            m_cursor += dt * m_playbackSpeed;

            // Khóa cursor ở snapshot cuối cùng
            if(m_cursor >= (float)m_snapshots.size() - 1.0f)
            {
                m_cursor = (float)m_snapshots.size() - 1.0f;
                m_isPlaying = false;
            }
        }

        bool isEmpty() const
        {
            return m_snapshots.empty();
        }

        int getSnapshotsCount() const
        {
            return m_snapshots.size();
        }

        // Lấy phần thập phân để tính nội suy (VD: 1.7 -> trả về 0.7)
        float getAlpha() const
        {
            return m_cursor - (int)m_cursor;
        }

        int getCurrentIdx() const
        {
            return (int)m_cursor;
        }

        int getNextIdx() const
        {
            return std::min((int)m_cursor + 1, (int)m_snapshots.size() - 1);
        }

        const Snapshot& getSnapshot(int index) const
        {
            return m_snapshots[index];
        }

        // Dùng cho tính năng Tua tiến/Tua lùi (Time Travel / Scrubbing)
        void seek(float targetStep)
        {
            if(m_snapshots.empty()) return;
            m_cursor = std::max(0.0f, std::min(targetStep, (float)m_snapshots.size() - 1.0f));
        }

        void play() { m_isPlaying = true; }
        void pause() { m_isPlaying = false; }

        bool isAtEnd() const
        {
            return m_snapshots.empty() || m_cursor >= (float)m_snapshots.size() - 1.0f;
        }

        void clear()
        {
            m_snapshots.clear();
            m_cursor = 0.0f;
            m_isPlaying = false;
        }
    };
}
}
