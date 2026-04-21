#pragma once
#include <vector>
#include <memory>
#include "ISnapshot.h"

namespace Core
{
    class HistoryManager
    {
    private:
        std::vector<std::shared_ptr<ISnapshot>> history;
        int currentIndex;

    public:
        HistoryManager()
        {
            currentIndex = -1;
        }

        // Thêm hàm này vào đây
        std::shared_ptr<ISnapshot> getSnapshotAt(int index) const
        {
            // Kiểm tra biên để tránh crash (Bounds checking)
            if(index < 0 || index >= (int)history.size())
            {
                return nullptr;
            }

            return history[index];
        }

        // Thêm một bản ghi mới vào lịch sử
        void addSnapshot(std::shared_ptr<ISnapshot> snapshot)
        {
            // Nếu người dùng đang tua lùi và có thao tác mới -> xóa bỏ tương lai cũ
            if(currentIndex < (int)history.size() - 1)
            {
                history.erase(history.begin() + currentIndex + 1, history.end());
            }

            history.push_back(snapshot);
            currentIndex++;
        }

        // Trả về bản ghi của bước liền trước (nếu có)
        std::shared_ptr<ISnapshot> goBack()
        {
            if(currentIndex > 0)
            {
                currentIndex--;
                return history[currentIndex];
            }
            return nullptr;
        }

        // Trả về bản ghi của bước liền sau (nếu có)
        std::shared_ptr<ISnapshot> goNext()
        {
            if(currentIndex < (int)history.size() - 1)
            {
                currentIndex++;
                return history[currentIndex];
            }
            return nullptr;
        }

        // Lấy bản ghi hiện tại
        std::shared_ptr<ISnapshot> getCurrent()
        {
            if(currentIndex >= 0 && currentIndex < (int)history.size())
            {
                return history[currentIndex];
            }
            return nullptr;
        }

        // Xóa toàn bộ lịch sử (Dùng khi reset hệ thống hoặc đổi thuật toán)
        void clear()
        {
            history.clear();
            currentIndex = -1;
        }

        // Dùng cho Dynamic Notch (Thanh Scrubber / Activity Metrics)
        int getCurrentIndex() const
        {
            return currentIndex;
        }

        int getTotalSnapshots() const
        {
            return (int)history.size();
        }
    };
}
