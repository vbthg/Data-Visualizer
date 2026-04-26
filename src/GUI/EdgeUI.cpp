#include "EdgeUI.h"
#include "Easing.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace GUI
{
    EdgeUI::EdgeUI(NodeUI* start, NodeUI* end, float thickness)
    {
        m_startNode = start;
        m_endNode = end;
        m_opacity = 1.f;

        m_isColorFilling = false;

        // Khởi tạo các biến nội suy
        m_baseThickness = thickness;
        m_currentThickness = thickness;
        m_targetThickness = thickness;

        m_baseColor = sf::Color(150, 150, 150); // Màu xám gốc của trạng thái tĩnh
        m_currentColor = m_baseColor;
        m_targetColor = m_baseColor;

        // Thêm vào cuối Constructor khởi tạo giá trị mặc định:
        m_fillProgress = 0.0f;
        m_fillFromStart = true;
        m_fillColor = sf::Color::White;
        m_baseFillColor = sf::Color(255, 255, 255, 0);

        m_isFocused = true;
        m_isPulsing = false;
        m_pulseProgress = 0.0f;
        m_pulseColor = sf::Color::Yellow;

        // Cài đặt lò xo cho dây đàn (Cứng và ít cản để nảy lâu hơn Node)
        m_offsetSpring.stiffness = 600.0f;
        m_offsetSpring.damping = 20.0f;
        m_offsetSpring.position = 0.0f;
        m_offsetSpring.target = 0.0f;
        m_offsetSpring.velocity = 0.0f;

        m_tSpring.stiffness = 400.0f;
        m_tSpring.damping = 25.0f;
        m_tSpring.snapTo(0.5f);

        m_vertices.setPrimitiveType(sf::TriangleStrip);
        m_weightText.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));
        m_weightText.setCharacterSize(15);
        m_weightText.setFillColor(sf::Color::White);
        // Tạo viền đen mỏng cho chữ để dễ đọc trên mọi nền
        m_weightText.setOutlineColor(sf::Color::Black);
        m_weightText.setOutlineThickness(1.0f);
    }

    NodeUI* EdgeUI::getStartNode() const
    {
        return m_startNode;
    }

    NodeUI* EdgeUI::getEndNode() const
    {
        return m_endNode;
    }

    void EdgeUI::applyState(const Core::EdgeState& state)
    {
        setFocusState(state.isFocused);

        // Cập nhật độ dày mục tiêu từ State
        m_targetThickness = state.thickness;

        m_isPulsing = state.isPulsing;
        m_pulseProgress = state.pulseProgress;
        m_pulseColor = state.pulseColor;

        m_isColorFilling = (state.fillProgress > 0.0f);
        m_fillProgress = state.fillProgress;
        m_baseFillColor = state.baseFillColor;
        m_fillColor = state.fillColor;
        m_fillFromStart = state.fillFromStart;

        m_opacity = state.opacity;

        // CẬP NHẬT TRỌNG SỐ (SUBTEXT)
        m_weightText.setString(state.subText);
        sf::FloatRect wBounds = m_weightText.getLocalBounds();
        m_weightText.setOrigin(wBounds.left + wBounds.width / 2.0f, wBounds.top + wBounds.height / 2.0f);
    }

    void EdgeUI::setFocusState(bool isFocused)
    {
        m_isFocused = isFocused;

        if (m_isFocused)
        {
            // Trở về trạng thái bình thường (Sáng rõ, độ dày gốc)
            m_targetThickness = m_baseThickness;
            m_targetColor = m_baseColor;
        }
        else
        {
            // Trạng thái Dim (Mờ đi 70% và độ dày co lại còn 4.0f)
            m_targetThickness = 4.0f;
            m_targetColor = sf::Color(80, 80, 80, 60); // Xám tối và trong suốt
        }
    }

    void EdgeUI::setTargetColor(sf::Color color)
    {
        m_targetColor = color;
    }

    void EdgeUI::setExactColor(sf::Color color)
    {
        m_currentColor = m_targetColor = color;
    }

    void EdgeUI::setMousePosition(sf::Vector2f pos)
    {
        m_mousePos = pos;
    }

    void EdgeUI::setPulse(float progress, sf::Color color)
    {
        m_isPulsing = true;
        m_pulseProgress = progress;
        m_pulseColor = color;
    }

    void EdgeUI::stopPulse()
    {
        m_isPulsing = false;
    }

    void EdgeUI::setColorFilling(float progress, sf::Color baseColor, sf::Color fillColor, bool fromStart)
    {
        m_isColorFilling = true;
        m_fillProgress = std::max(0.0f, std::min(1.0f, progress));
        m_baseFillColor = baseColor;
        m_fillColor = fillColor;
        m_fillFromStart = fromStart;
    }

    void EdgeUI::clearColorFilling()
    {
        m_isColorFilling = false;
        m_fillProgress = 0.0f;
    }

    void EdgeUI::update(float dt)
    {
        // 1. NỘI SUY (LERP) ĐỘ DÀY VÀ MÀU SẮC
        float lerpSpeed = 10.0f; // Tốc độ chuyển đổi (càng cao càng nhanh)
        m_currentThickness = m_targetThickness;
//        m_currentThickness += (m_targetThickness - m_currentThickness) * (lerpSpeed * dt);

        // Lerp từng kênh màu (R, G, B, A) một cách thủ công để an toàn
        float r = m_currentColor.r + (m_targetColor.r - m_currentColor.r) * (lerpSpeed * dt);
        float g = m_currentColor.g + (m_targetColor.g - m_currentColor.g) * (lerpSpeed * dt);
        float b = m_currentColor.b + (m_targetColor.b - m_currentColor.b) * (lerpSpeed * dt);
        float a = m_currentColor.a + (m_targetColor.a - m_currentColor.a) * (lerpSpeed * dt);
        m_currentColor = sf::Color(static_cast<sf::Uint8>(r),
                                   static_cast<sf::Uint8>(g),
                                   static_cast<sf::Uint8>(b),
                                   static_cast<sf::Uint8>(a * m_opacity));

        sf::Vector2f p1 = m_startNode->getCurrentPosition();
        sf::Vector2f p2 = m_endNode->getCurrentPosition();

        sf::Vector2f dir = p2 - p1;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        float targetOffset = 0.0f;
        float targetT = 0.5f; // Mặc định uốn ở giữa

        if(len > 0.0001f)
        {
            sf::Vector2f dirNorm = dir / len;
            sf::Vector2f normal(-dirNorm.y, dirNorm.x);

            // --- 1. TÍNH LỰC CẢN QUÁN TÍNH (KINETIC DRAG) ---
            sf::Vector2f v1 = m_startNode->getVelocity();
            sf::Vector2f v2 = m_endNode->getVelocity();
            sf::Vector2f avgVel = (v1 + v2) / 2.0f;

            // Tích vô hướng (Dot Product) lấy phần vận tốc đẩy ngang dây
            float sidewaysVel = avgVel.x * normal.x + avgVel.y * normal.y;
            targetOffset += sidewaysVel * -0.06f; // Hệ số cản gió

            if(m_isFocused)
            {
                // --- 2. TÍNH LỰC HÚT TỪ CHUỘT (MAGNETIC HOVER) ---
                // Chiếu điểm chuột lên đoạn thẳng để tìm khoảng cách
                float t = ((m_mousePos.x - p1.x) * dir.x + (m_mousePos.y - p1.y) * dir.y) / (len * len);
                t = std::max(0.0f, std::min(1.0f, t));

                sf::Vector2f projection = p1 + t * dir;
                sf::Vector2f distVec = m_mousePos - projection;
                float dist = std::sqrt(distVec.x * distVec.x + distVec.y * distVec.y);

                float hoverRadius = 40.0f; // Vùng bắt từ tính
                if(dist < hoverRadius)
                {
                    // Dùng Cross Product để xem chuột đang ở bờ trái hay phải của dây
                    float cross = dirNorm.x * distVec.y - dirNorm.y * distVec.x;
                    float sign = (cross > 0.0f) ? -1.0f : 1.0f;

                    // Công thức tỷ lệ nghịch: Càng sát dây kéo càng mạnh (Tối đa 40px)
                    float pullStrength = std::pow(1.0f - (dist / hoverRadius), 2.0f) * 40.0f;

                    targetOffset += sign * pullStrength;
                    targetT = t; // Khi chuột vào vùng, dời điểm uốn tới vị trí chuột
                }
            }

            sf::Vector2f anchorPoint = p1 + dir * m_tSpring.position;
            sf::Vector2f controlPoint = anchorPoint + normal * m_offsetSpring.position;

            // Công thức Bezier tại t = 0.5: P = 0.25*P1 + 0.5*CP + 0.25*P2
            sf::Vector2f midPath = 0.25f * p1 + 0.5f * controlPoint + 0.25f * p2;

            // Đẩy chữ lên trên dây một chút để không bị đè
            m_weightText.setPosition(midPath + normal * 15.f);
        }

        // Cập nhật độ mờ cho chữ
        sf::Color wCol = m_weightText.getFillColor();
        wCol.a = static_cast<sf::Uint8>(m_opacity * 255.f);
        m_weightText.setFillColor(wCol);

        sf::Color wOutCol = m_weightText.getOutlineColor();
        wOutCol.a = wCol.a;
        m_weightText.setOutlineColor(wOutCol);


        // Cập nhật lò xo đàn hồi
        m_offsetSpring.target = targetOffset;
        m_offsetSpring.update(dt);

        m_tSpring.target = targetT;
        m_tSpring.update(dt);

        recalculateVertices();
    }

    void EdgeUI::recalculateVertices()
    {
        sf::Vector2f startPos = m_startNode->getCurrentPosition();
        sf::Vector2f endPos = m_endNode->getCurrentPosition();

        sf::Vector2f dir = endPos - startPos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if(len == 0.0f) return;

        sf::Vector2f normal(-dir.y / len, dir.x / len);
        sf::Vector2f midPoint = (startPos + endPos) / 2.0f;

        // Tìm điểm neo trên đoạn thẳng dựa vào lò xo T
        sf::Vector2f anchorPoint = startPos + dir * m_tSpring.position;

        // Dịch Control Point bằng chính giá trị đang nhún nhảy của Lò xo
        // Đẩy điểm Control Point ra xa khỏi điểm neo
        sf::Vector2f controlPoint = anchorPoint + normal * m_offsetSpring.position;
//        sf::Vector2f controlPoint = midPoint + normal * m_offsetSpring.position;

        int segments = 1;
        if(m_isColorFilling || m_isPulsing)
        {
            segments = static_cast<int>(std::clamp(len / 30.0f, 15.0f, 100.0f));
        }
        m_vertices.resize((segments + 1) * 2);

        sf::Vector2f prevP = startPos;

        for(int i = 0; i <= segments; ++i)
        {
            float t = (float)i / segments;
            float u = 1.0f - t;

            // Bezier Bậc 2
            sf::Vector2f p = u * u * startPos + 2.0f * u * t * controlPoint + t * t * endPos;

            sf::Vector2f tangent = p - prevP;
            float tangentLen = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);

            sf::Vector2f pNormal = normal; // Fallback
            if(tangentLen > 0.0f)
            {
                pNormal = sf::Vector2f(-tangent.y / tangentLen, tangent.x / tangentLen);
            }

            // --- TÍNH TOÁN HIỆU ỨNG HẠT SÁNG (DATA PULSE) ---
            sf::Color vertexColor = m_currentColor;
            vertexColor.a *= m_opacity;

            // ÁP DỤNG HIỆU ỨNG FILL/MỌC DÂY (Ghi đè màu dây nếu progress > 0)
            if (m_isColorFilling)
            {
                // Đảo ngược trục t nếu lan từ dưới lên
                float normalizedT = m_fillFromStart ? t : (1.0f - t);

                // Độ rộng của rìa mềm (20% chiều dài dây)
                float softEdgeWidth = std::clamp(40.0f / len, 0.01f, 0.5f);

                if (normalizedT <= m_fillProgress - softEdgeWidth)
                {
                    vertexColor = m_fillColor; // Vùng đã lan xong 100%
                }
                else if (normalizedT < m_fillProgress)
                {
                    // Vùng gradient (Rìa mềm)
                    float intensity = (m_fillProgress - normalizedT) / softEdgeWidth;

                    // Thuật toán Smoothstep làm mượt sự chuyển đổi
                    intensity = intensity * intensity * (3.0f - 2.0f * intensity);

                    // Nội suy từng kênh màu giữa màu nền và màu phủ
                    float r = m_baseFillColor.r + (m_fillColor.r - m_baseFillColor.r) * intensity;
                    float g = m_baseFillColor.g + (m_fillColor.g - m_baseFillColor.g) * intensity;
                    float b = m_baseFillColor.b + (m_fillColor.b - m_baseFillColor.b) * intensity;
                    float a = m_baseFillColor.a + (m_fillColor.a - m_baseFillColor.a) * intensity;

                    vertexColor = sf::Color(static_cast<sf::Uint8>(r),
                                            static_cast<sf::Uint8>(g),
                                            static_cast<sf::Uint8>(b),
                                            static_cast<sf::Uint8>(a));
                }
                else
                {
                    vertexColor = m_baseFillColor; // Vùng chưa lan tới
                }
            }

            if(m_isPulsing)
            {
//                std::cout << "[PULSE PROGRESS]: " << m_pulseProgress << "\n";
                float dist = std::abs(t - m_pulseProgress);
                float pulsePixelWidth = 60.0f; // Độ rộng vệt sáng cố định 60px
                float pulseWidth = pulsePixelWidth / len;

                if(dist < pulseWidth)
                {
                    // Công thức Smoothstep: Tạo vùng gradient cực kỳ mượt mà ở rìa hạt sáng
                    float intensity = 1.0f - (dist / pulseWidth);
                    intensity = intensity * intensity * (3.0f - 2.0f * intensity);

                    vertexColor = Utils::Math::Easing::lerpColor(m_currentColor, m_pulseColor, intensity);
                }
            }

            m_vertices[i * 2].position = p + pNormal * (m_currentThickness / 2.0f);
            m_vertices[i * 2].color = vertexColor;

            m_vertices[i * 2 + 1].position = p - pNormal * (m_currentThickness / 2.0f);
            m_vertices[i * 2 + 1].color = vertexColor;

            prevP = p;
        }
    }

    void EdgeUI::draw(sf::RenderTarget& target) const
    {
        target.draw(m_vertices);
        if(!m_weightText.getString().isEmpty() && m_opacity > 0.1f)
        {
            target.draw(m_weightText);
        }
    }
}
