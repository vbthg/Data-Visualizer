#pragma once
#include <SFML/Graphics.hpp>
#include "NodeUI.h"
#include "Spring.h"
#include "AnimationMetadata.h"

namespace GUI
{
    class EdgeUI
    {
    private:
        NodeUI* m_startNode;
        NodeUI* m_endNode;

        sf::VertexArray m_vertices;
        // --- CÁC BIẾN QUẢN LÝ ĐỘ DÀY & MÀU SẮC (LERP) ---
        float m_baseThickness;
        float m_currentThickness;
        float m_targetThickness;

        sf::Color m_baseColor;
        sf::Color m_currentColor;
        sf::Color m_targetColor;

        // --- TRẠNG THÁI FOCUS ---
        bool m_isFocused;

        // --- 1. ĐỘNG CƠ VẬT LÝ (DRAG & HOVER) ---
        Utils::Physics::Spring m_offsetSpring;
        Utils::Physics::Spring m_tSpring;
        sf::Vector2f m_mousePos;

        // --- 2. ĐỘNG CƠ HẠT SÁNG (DATA PULSE) ---
        bool m_isPulsing;
        float m_pulseProgress; // Từ 0.0f (Đầu) đến 1.0f (Đích)
        sf::Color m_pulseColor;


        // --- ĐỘNG CƠ FILL MÀU / MỌC DÂY ---
        float m_fillProgress;
        bool m_fillFromStart;
        sf::Color m_fillColor;
        sf::Color m_baseFillColor; // Màu nền trước khi lan tới (có thể set Alpha = 0 để làm tàng hình)
        bool m_isColorFilling;


        float m_opacity;


        void recalculateVertices();

    public:
        // Cạnh mặc định luôn thẳng (không cần truyền baseCurveOffset nữa)
        EdgeUI(NodeUI* start, NodeUI* end, float thickness = 3.0f);

        // Lấy 2 Node ở 2 đầu để Heap có thể kiểm tra xem cạnh này nối với ai
        NodeUI* getStartNode() const;
        NodeUI* getEndNode() const;

        // --- HÀM MỚI ĐỂ QUẢN LÝ TRẠNG THÁI ---
        void applyState(const Core::EdgeState& state);
        void setFocusState(bool isFocused);

        // --- CÁC HÀM TƯƠNG TÁC ---
        void setMousePosition(sf::Vector2f pos); // Liên tục nạp vị trí chuột vào đây
        void setTargetColor(sf::Color color);
        void setExactColor(sf::Color color);

        // Kích hoạt hạt sáng chạy trên dây (truyền t từ AnimationManager)
        void setPulse(float progress, sf::Color color = sf::Color::Yellow);
        void stopPulse(); // Dọn dẹp hiệu ứng khi chạy xong

        // Hàm kích hoạt hiệu ứng lan màu/mọc dây
        void setColorFilling(float progress, sf::Color baseColor, sf::Color fillColor, bool fromStart = true);
        // Hàm reset về trạng thái bình thường
        void clearColorFilling();

        void update(float dt);
        void draw(sf::RenderTarget& target) const;
    };
}
