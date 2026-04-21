#include "GUI/Scrollbar.h"
#include "ViewHandler.h" // Đảm bảo include để nhất quán logic

namespace GUI
{
Scrollbar::Scrollbar()
{
    m_thicknessSpring.snapTo(4.f);
    m_thicknessSpring.stiffness = 400.f;
    m_thicknessSpring.damping = 30.f;

    m_thumb.setFillColor(sf::Color(255, 255, 255, 0));
    m_thumb.setRadius(2.f);
}

void Scrollbar::show()
{
    m_alpha = 1.f;
    m_hideTimer = 2.0f;
}

void Scrollbar::update(float dt, float currentScroll, float contentHeight, float viewportHeight, const sf::Vector2f& localMouse)
{
    // 1. CLAMP LOGIC: Nếu nội dung không đủ để cuộn thì không hiện thanh cuộn
    if(contentHeight <= viewportHeight)
    {
        m_alpha = 0.f;
        return;
    }

    // 2. Logic ẩn hiện (mờ dần theo thời gian)
    if(m_hideTimer > 0.f)
    {
        m_hideTimer -= dt;
    }
    else if(m_alpha > 0.f)
    {
        m_alpha -= dt * 2.f;
    }
    m_alpha = std::max(0.f, m_alpha);

    // 3. Tính toán kích thước chuẩn (Apple style: thumb tỉ lệ với nội dung)
    float viewRatio = viewportHeight / contentHeight;
    float thumbHeight = viewportHeight * viewRatio;

    // Giới hạn chiều dài thumb không quá ngắn (min) và không quá dài (max 90% viewport)
    thumbHeight = std::max(m_minThumbHeight, std::min(thumbHeight, viewportHeight * 0.9f));


    float maxScroll = contentHeight - viewportHeight;

    // 4. RUBBER BAND LOGIC (Xử lý nén và vị trí khi overscroll)
    float overscroll = 0.f;
    float thumbY = 0.f;
    float displayedHeight = thumbHeight;


    if(currentScroll < 0.f) // Vượt biên trên
    {
        overscroll = -currentScroll;
        displayedHeight = std::max(10.f, thumbHeight - overscroll);
        thumbY = 0.f; // Dính ở đỉnh và co lại từ dưới lên
    }
    else if(currentScroll > maxScroll) // Vượt biên dưới
    {
        overscroll = currentScroll - maxScroll;
        displayedHeight = std::max(10.f, thumbHeight - overscroll);
        thumbY = viewportHeight - displayedHeight; // Dính ở đáy và co lại từ trên xuống
    }
    else // Trạng thái cuộn bình thường trong biên
    {
        float scrollRatio = currentScroll / maxScroll;
        thumbY = scrollRatio * (viewportHeight - thumbHeight);
    }


    // 5. HOVER EFFECT (Dùng ViewHandler's localMouse)
    sf::FloatRect trackArea(350.f - 15.f, 0.f, 15.f, viewportHeight);
    m_isHovered = trackArea.contains(localMouse);
    m_thicknessSpring.target = (m_isHovered || m_isDragging) ? 8.f : 4.f;
    m_thicknessSpring.update(dt);

    if(m_isHovered)
    {
        show();
    }

    // GÁN VÀO BIẾN THÀNH VIÊN Ở ĐÂY
    m_currentThumbHeight = displayedHeight;

    // 6. CẬP NHẬT VISUAL
    m_thumb.setSize({m_thicknessSpring.position, displayedHeight});
    m_thumb.setRadius(m_thicknessSpring.position / 2.f);
    m_thumb.setOrigin(m_thicknessSpring.position, 0.f);
    m_thumb.setPosition(350.f - 4.f, thumbY);

    sf::Uint8 a = static_cast<sf::Uint8>(m_alpha * 180.f);
    m_thumb.setFillColor(sf::Color(255, 255, 255, a));
}

void Scrollbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if(m_alpha > 0.01f)
    {
        target.draw(m_thumb, states);
    }
}
}
