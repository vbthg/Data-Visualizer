#pragma once

namespace GUI
{
    // 3 Kích thước cố định (Khóa khung viền - Tránh co giật)
    enum class NotchSize
    {
        Compact,    // Nghỉ ngơi / Chờ (Chỉ hiện icon)
        Standard,   // Đang chạy thuật toán (Chứa Text và Waveform)
        Expanded,   // Cảnh báo / Lỗi / Chờ tương tác (To, rõ ràng)
        Tray
    };

    // Các kịch bản ngữ cảnh dành cho thuật toán gọi
    enum class Scenario
    {
        Initial,        // Mới vào app, chưa có dữ liệu
        Idle,           // Trạng thái nghỉ (đã có dữ liệu)
        Processing,     // Đang duyệt, đang so sánh (Nhảy Waveform)
        Success,        // Hoàn thành xuất sắc (Màu xanh, tự thu hồi)
        Error,          // Lỗi (Màu đỏ, rung lắc, tự thu hồi chậm)
        Warning,        // Cảnh báo nhẹ
        AwaitingInput,  // Dừng lại chờ người dùng click chuột
        FileTray
    };

}
