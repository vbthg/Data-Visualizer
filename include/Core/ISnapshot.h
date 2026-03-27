#pragma once

namespace Core
{
    // Interface trừu tượng. Các Data Structure (AVL, R-Tree...) sẽ kế thừa class này
    // để định nghĩa cấu trúc lưu trữ dữ liệu riêng của chúng tại một thời điểm.
    class ISnapshot
    {
    public:
        virtual ~ISnapshot() = default;
    };
}
