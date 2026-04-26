#include "SpeedController.h"
#include "ViewHandler.h"
#include <iostream>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    SpeedController::SpeedController(const sf::Font& font, Core::TimelineManager* timeline)
        : m_mainButton(font, "1x", {50.f, 50.f}), // Kích thước nút mặc định
          m_slider(Theme::Style::SpeedSliderWidth),                         // Chiều dài track của slider
          m_timeline(timeline)
    {
        m_isExpanded = false;
        m_isPressing = false;
        m_pressTimer = 0.0f;

        m_mainButton.setMaxScale(1.1f);
        m_mainButton.setCornerRadius(20.f);

        // Khởi tạo các mốc tốc độ nhanh
        m_quickSpeeds = {0.25f, 1.0f, 2.0f, 4.0f};
        m_currentSpeedIdx = 1; // Mặc định là 1.0x

        // Thiết lập kích thước
        m_collapsedWidth = m_mainButton.getSize().x;
        // Chiều rộng khi mở = Nút + Khoảng cách (15px) + Slider
        m_expandedWidth = m_collapsedWidth + Theme::Style::SpeedControllerGap + Theme::Style::SpeedSliderWidth;

        // Cấu hình Spring (Đầm chắc chuẩn Apple)
        m_widthSpring.position = m_collapsedWidth;
        m_widthSpring.target = m_collapsedWidth;
        m_widthSpring.stiffness = Theme::Animation::MorphStiffness;
        m_widthSpring.damping = Theme::Animation::MorphDamping;

        // Cấu hình Slider
        m_slider.setRange(0.1f, 4.0f, 0.1f);
        m_slider.setValue(1.0f, true); // Đặt giá trị ngay lập tức
        m_slider.setOpacity(0.0f);     // Ẩn ban đầu

        // Liên kết dữ liệu Slider -> Text của Button
        m_slider.onValueChanged = [this](float val)
        {
            // 1. Hiển thị text lên Button (luôn đồng bộ dù đang thu gọn hay mở rộng)
            std::string text = std::to_string(val);
            text.erase(text.find_last_not_of('0') + 1, std::string::npos);
            if(text.back() == '.') text.pop_back();
            m_mainButton.setText(text + "x");
            m_mainButton.triggerTextPop();

            // 2. Đồng bộ trực tiếp với Timeline
            if(m_timeline)
            {
                m_timeline->setPlaybackSpeed(val * 1.5f);
            }
        };
    }

    void SpeedController::cycleSpeed()
    {
        float currentVal = m_slider.getValue();
        int nextIdx = 0;

        // Tìm index của mốc quickSpeed lớn nhất mà vẫn <= currentVal
        // Anh duyệt ngược từ cuối mảng để tìm mốc cao nhất thỏa mãn
        for(int i = (int)m_quickSpeeds.size() - 1; i >= 0; --i)
        {
            if(currentVal >= m_quickSpeeds[i])
            {
                // Mốc tiếp theo sẽ là index hiện tại + 1 (vòng lặp %)
                nextIdx = (i + 1) % m_quickSpeeds.size();
                break;
            }

            // Trường hợp đặc biệt: nếu currentVal nhỏ hơn cả mốc nhỏ nhất (0.1x < 0.25x)
            if(i == 0 && currentVal < m_quickSpeeds[0])
            {
                nextIdx = 0;
            }
        }

        float newSpeed = m_quickSpeeds[nextIdx];

        // Slider.setValue sẽ tự động gọi callback onValueChanged để cập nhật UI & Timeline
        m_slider.setValue(newSpeed);
    }

    void SpeedController::collapse()
    {
        m_isExpanded = false;
        m_widthSpring.target = m_collapsedWidth;
        m_slider.setOpacity(0.0f);
        m_mainButton.setScaleTarget(1.f);
    }

    bool SpeedController::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        bool handled = false;

        // 1. Nếu đang mở, ưu tiên cho Slider bắt sự kiện
        if(m_isExpanded)
        {
            if(m_slider.handleEvent(event, window)) handled = true;

            // Logic Click Outside để đóng
            if(event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i mousePos = {event.mouseButton.x, event.mouseButton.y};

                // Vùng hcn ảo bao quanh SpeedController
                sf::FloatRect bounds(m_position.x - m_widthSpring.position / 2.0f,
                                     m_position.y - 25.0f,
                                     m_widthSpring.position, 50.0f);

                if(!Utils::ViewHandler::isMouseInFrame(mousePos, window, bounds))
                {
                    collapse();
                    // Trả về true vì cú click này đã dùng để đóng controller,
                    // không nên cho phép click trúng Node phía dưới.
                    handled = true;
                }
            }
        }

        // 2. Button luôn được phép bắt sự kiện hover/click
        if(m_mainButton.handleEvent(event, window)) handled = true;

        // 3. Cỗ máy trạng thái Nhấn giữ (Hold)
        if(event.type == sf::Event::MouseButtonPressed)
        {
            if(event.mouseButton.button == sf::Mouse::Left)
            {
                if(m_mainButton.isHovering())
                {
                    m_isPressing = true;
                    m_pressTimer = 0.0f;
                    handled = true;
                }
            }
            else if(event.mouseButton.button == sf::Mouse::Right)
            {
                // Bấm chuột phải để thao tác nhanh Expand/Collapse
                if(m_mainButton.isHovering())
                {
                    m_isExpanded = !m_isExpanded;
                    m_widthSpring.target = m_isExpanded ? m_expandedWidth : m_collapsedWidth;
                    handled = true;
                }
            }
        }
        else if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            if(m_isPressing)
            {
                m_isPressing = false;

                // Nếu thả chuột ra nhanh (dưới 0.5s) và đang không mở rộng -> Đổi tốc độ
                if(m_pressTimer < Theme::Animation::HoldDelay && !m_isExpanded && m_mainButton.isHovering())
                {
                    cycleSpeed();
                }
                handled = true; // Kết thúc chu kỳ nhấn chuột trên component
            }
        }

        return handled;
    }

    void SpeedController::update(sf::RenderWindow& window, float dt)
    {
        // 1. Kiểm tra thời gian Hold
        if(m_isPressing && !m_isExpanded)
        {
            m_pressTimer += dt;

            if(m_pressTimer >= 0.3f && m_pressTimer < Theme::Animation::HoldDelay)
            {
                m_mainButton.setScaleTarget(1.22f);
            }

            if(m_pressTimer >= Theme::Animation::HoldDelay)
            {
                m_isExpanded = true;
                m_isPressing = false;
                m_widthSpring.target = m_expandedWidth;
                m_mainButton.setScaleTarget(1.22f);
//                m_pressTimer = 0;
            }
        }

        // 2. Cập nhật lò xo bề ngang
        m_widthSpring.update(dt);

        // 3. Tính toán tiến trình biến hình (Morphing Progress: 0.0 -> 1.0)
        float progress = (m_widthSpring.position - m_collapsedWidth) / (m_expandedWidth - m_collapsedWidth);

        // Tránh lỗi chia cho 0 hoặc sai số nhỏ
        if(progress < 0.0f) progress = 0.0f;
        if(progress > 1.0f) progress = 1.0f;

        float sliderOpacityProgress = 0.0f;
        if(progress > 0.3f)
        {
            sliderOpacityProgress = (progress - 0.3f) / 0.7f;
        }

        // 4. Liên kết Opacity của Slider với tiến trình nở ra
        // Khi progress = 1.0 (mở hết cỡ), Slider rõ 100% (255)
        m_slider.setOpacity(sliderOpacityProgress * 255.0f);

        // 5. Cập nhật vị trí các thành phần con
        // Để Button nằm im một chỗ khi nở, ta neo nó vào cạnh trái của khung Controller
        float leftEdge = m_position.x - m_widthSpring.position / 2.0f;

        m_mainButton.setPosition({leftEdge + m_collapsedWidth / 2.0f, m_position.y});
        m_mainButton.update(window, dt);

        if(progress > 0.01f) // Chỉ update Slider khi nó có hiển thị
        {
            // Đặt Slider nằm kế bên phải Button
            m_slider.setPosition({leftEdge + m_collapsedWidth + 15.0f, m_position.y});
            m_slider.update(window, dt);
        }
    }

    void SpeedController::draw(sf::RenderTarget& target)
    {
        m_mainButton.draw(target);

        // Tối ưu Draw Call: Chỉ vẽ khi có độ mờ > 0
        if(m_widthSpring.position > m_collapsedWidth + 1.0f)
        {
            m_slider.draw(target);
        }
    }

    void SpeedController::setPosition(const sf::Vector2f pos)
    {
        m_position = pos;
    }

    sf::Vector2f SpeedController::getPosition() const
    {
        return m_position;
    }

    float SpeedController::getWidth() const
    {
        // QUAN TRỌNG: Dock sẽ liên tục gọi hàm này.
        // Khi spring nảy, bề ngang thay đổi, Dock sẽ đẩy các nút khác dãn ra cực kỳ mượt mà.
        return m_widthSpring.position;
    }
}
