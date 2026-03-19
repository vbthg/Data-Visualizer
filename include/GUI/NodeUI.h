#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Spring.h"
#include "Easing.h"

namespace GUI
{
    class NodeUI
    {
    private:
        // --- THÀNH PHẦN HIỂN THỊ (VISUALS) ---
        sf::CircleShape m_shape; // Hình dáng của Node (có thể thay bằng Squircle nếu muốn)
        sf::Text m_valueText;    // Chữ số hiển thị bên trong Node
        sf::Font* m_font;        // Con trỏ lưu Font chữ (để không phải load lại font nhiều lần)

        sf::Vector2f m_arcOffset;

        // --- HỆ THỐNG VẬT LÝ (PHYSICS SPRINGS) ---
        // Lò xo giúp tạo chuyển động mượt mà và có quán tính
        Utils::Physics::Spring m_xSpring;     // Lò xo quản lý tọa độ X
        Utils::Physics::Spring m_ySpring;     // Lò xo quản lý tọa độ Y
        Utils::Physics::Spring m_scaleSpring; // Lò xo quản lý độ lớn (phóng to/thu nhỏ)

        // --- HỆ THỐNG MÀU SẮC (COLOR MORPHING) ---
        // Tách biệt giữa màu hiện tại (Current) và màu đích đến (Target) để tạo hiệu ứng chuyển màu mượt mà
        sf::Color m_currentColor;
        sf::Color m_targetColor;

        sf::Color m_currentTextColor;
        sf::Color m_targetTextColor;

        sf::Color m_currentOutlineColor;
        sf::Color m_targetOutlineColor;

    public:
        // Khởi tạo Node với font chữ và bán kính mặc định là 25.0f
        NodeUI(sf::Font* font, float radius = 25.0f);

        // Cập nhật giá trị hiển thị (VD: "15", "99") và tự động căn giữa chữ
        void setValue(const std::string& val);

        // --- CÁC HÀM ĐIỀU KHIỂN CHUYỂN ĐỘNG ---
        // Di chuyển mượt mà: Giao tọa độ đích cho lò xo tự kéo đến
        void setTargetPosition(float x, float y);

        // Di chuyển tức thời (Teleport): Ép Node đến ngay lập tức, triệt tiêu mọi lực quán tính của lò xo
        void setExactPosition(float x, float y);

        // --- CÁC HÀM ĐIỀU KHIỂN MÀU SẮC ---
        // Chuyển màu mượt mà: Đặt màu đích, hàm update() sẽ tự động pha màu dần dần
        void setTargetColor(sf::Color bgTarget, sf::Color textTarget, sf::Color outlineTarget);

        // Đổi màu tức thời: Không có hiệu ứng chuyển tiếp
        void setExactColor(sf::Color bg, sf::Color text, sf::Color outline);

        // Hiệu ứng phóng to/thu nhỏ mượt mà (VD: truyền 1.2f để bự lên khi đang được thuật toán xét)
        void setTargetScale(float scale);

        // --- CÁC HÀM CUNG CẤP DỮ LIỆU CHO LỚP KHÁC (VD: EdgeUI) ---
        sf::Vector2f getCurrentPosition() const; // Lấy tọa độ thực tế tại khung hình này
        sf::Vector2f getVelocity() const;        // Lấy vận tốc hiện tại để EdgeUI làm hiệu ứng cong đuôi (Drag)
        sf::Vector2f getTargetPosition() const;

        // --- VÒNG LẶP CHÍNH ---
        void update(float dt);                   // Tính toán vật lý và nội suy màu sắc (Gọi 60 lần/giây)
        void draw(sf::RenderTarget& target) const; // Vẽ ra màn hình

        // Thêm vào phần public của NodeUI.h:
        void setArcOffset(sf::Vector2f offset);
        sf::Vector2f getArcOffset() const;
    };
}
