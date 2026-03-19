#include "DynamicIsland.h"
#include "Theme.h"
#include <algorithm>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    DynamicIsland::DynamicIsland(const sf::Font& font)
        : currentState(State::Idle), displayTimer(0.0f)
    {
        // 1. Giao diện chuẩn "Apple Dynamic Island" (Nền đen, chữ trắng)
        background.setFillColor(sf::Color(0, 0, 0, 220)); // Đen hơi trong suốt

        logText.setFont(font);
        logText.setCharacterSize(16);
        logText.setFillColor(sf::Color::White);

        // 2. Setup Vật lý Morphing
        float stiff = Theme::Animation::MorphStiffness;
        float damp = Theme::Animation::MorphDamping;

        wSpring.stiffness = stiff; wSpring.damping = damp;
        hSpring.stiffness = stiff; hSpring.damping = damp;

        // Thêm setup lò xo Y
        ySpring.stiffness = Theme::Animation::SlideStiffness;
        ySpring.damping = Theme::Animation::SlideDamping;

        // Alpha nảy lẹ hơn một chút để chữ không bị mờ quá lâu
        alphaSpring.stiffness = 300.0f;
        alphaSpring.damping = 25.0f;

        // 3. Khởi tạo trạng thái Idle (Viên thuốc nhỏ 40x40)
        wSpring.snapTo(100.0f);
        hSpring.snapTo(35.0f);
        alphaSpring.snapTo(0.0f);
    }

    DynamicIsland::~DynamicIsland()
    {
    }

    void DynamicIsland::setPosition(float x, float y)
    {
        position = {x, y};
    }

//    void DynamicIsland::pushMessage(const std::string& msg)
//    {
//        messageQueue.push(msg);
//
//        // Nếu đang ngủ đông thì đánh thức nó dậy ngay
//        if (currentState == State::Idle)
//        {
//            currentMessage = messageQueue.front();
//            messageQueue.pop();
//
//            logText.setString(currentMessage);
//            calculateTargetBounds();
//
//            currentState = State::Expanding;
//            alphaSpring.target = 255.0f;
//        }
//    }

    void DynamicIsland::showMessage(const std::string& msg)
    {
        currentMessage = msg;
        logText.setString(currentMessage);
        calculateTargetBounds(); // Tính lại size cái hộp

        currentState = State::Expanding;
        alphaSpring.target = 255.0f; // Sáng chữ lên
    }

    void DynamicIsland::hideMessage()
    {
        // Khi gọi hàm này, hộp chữ sẽ mờ đi và thu gọn lại thành viên thuốc
        currentState = State::FadingOut;
        alphaSpring.target = 0.0f;
    }

    void DynamicIsland::clear()
    {
        // Dọn sạch hàng đợi
//        std::queue<std::string> empty;
//        std::swap(messageQueue, empty);

        // Thu hồi về trạng thái ngủ ngay lập tức
        currentState = State::FadingOut;
        alphaSpring.target = 0.0f;
    }

    void DynamicIsland::calculateTargetBounds()
    {
        // Tính toán vùng chữ thực tế
        sf::FloatRect bounds = logText.getLocalBounds();

        // Căn lại Origin của Text về chính giữa
        logText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);

        // Padding (Khoảng cách từ chữ ra viền)
        float paddingX = 40.0f;
        float paddingY = 20.0f;

        // Nếu chuỗi rỗng (Idle) thì mục tiêu là viên thuốc 40x40
        if (currentMessage.empty())
        {
            wSpring.target = 100.0f;
            hSpring.target = 35.0f;
        }
        else
        {
            wSpring.target = bounds.width + paddingX;
            hSpring.target = std::max(40.0f, bounds.height + paddingY); // Chiều cao tối thiểu là 40
        }
    }

    void DynamicIsland::setX(float x)
    {
        position.x = x;
    }

    void DynamicIsland::setTargetY(float targetY)
    {
        ySpring.target = targetY;
    }

    void DynamicIsland::snapToY(float startY)
    {
        ySpring.snapTo(startY);
        position.y = startY;
    }

    void DynamicIsland::update(float dt)
    {
        // Cập nhật lò xo trượt dọc
        ySpring.update(dt);
        position.y = ySpring.position;

        // 1. Cập nhật lò xo
        wSpring.update(dt);
        hSpring.update(dt);
        alphaSpring.update(dt);

        // 2. Máy trạng thái (State Machine)
        switch (currentState)
        {
            case State::Expanding:
            {
                // Khi lò xo chữ (Alpha) đã nở gần xong -> Chuyển sang đọc
//                if (alphaSpring.position > 240.0f)
//                {
//                    currentState = State::Reading;
//                    displayTimer = 0.0f;
//                }
                break;
            }
//            case State::Reading:
//            {
//                displayTimer += dt;
//                // Hết thời gian đọc -> Bắt đầu mờ đi
//                if (displayTimer >= DisplayDuration)
//                {
//                    currentState = State::FadingOut;
//                    alphaSpring.target = 0.0f;
//                }
//                break;
//            }
            case State::FadingOut:
            {
                // Khi chữ đã mờ hẳn (Alpha < 10) -> Xử lý tiếp hàng đợi
                if (alphaSpring.position < 10.0f)
                {
//                    if (!messageQueue.empty())
//                    {
//                        // Lấy câu tiếp theo ra
//                        currentMessage = messageQueue.front();
//                        messageQueue.pop();
//
//                        logText.setString(currentMessage);
//                        calculateTargetBounds(); // Tính lại lò xo kích thước
//
//                        currentState = State::Expanding;
//                        alphaSpring.target = 255.0f; // Sáng chữ lên lại
//                    }
//                    else
//                    {
//                        // Hết việc -> Thu nhỏ thành viên thuốc tàng hình
//                        currentMessage = "";
//                        calculateTargetBounds();
//                        currentState = State::Idle;
//                    }

                    currentMessage = "";
                    calculateTargetBounds();
                    currentState = State::Idle;
                }
                break;
            }
            case State::Idle:
            default:
                break;
        }

        // 3. Cập nhật hình ảnh (Visual Layout)
        // Luôn bo tròn 2 đầu bằng một nửa chiều cao
        float currentH = hSpring.position;
        background.setSize({wSpring.position, currentH});
        // Set bán kính bo góc = exacly 1/2 chiều cao để tạo hình bán nguyệt hoàn hảo ở 2 đầu
        background.setRadius(currentH / 2.0f); // (Sửa thành setCornerRadius nếu class của bạn dùng tên đó)
        background.setOrigin(wSpring.position / 2.0f, currentH / 2.0f);

        // Đặt ở vị trí neo (VD: giữa viền trên)
        background.setPosition(position);
        logText.setPosition(position);

        // Áp dụng Alpha
        sf::Color tColor = logText.getFillColor();
        tColor.a = (sf::Uint8)std::max(0.0f, std::min(alphaSpring.position, 255.0f));
        logText.setFillColor(tColor);

        // Khi Idle, làm mờ luôn cái nền đen đi cho sạch màn hình
//        sf::Color bgColor = background.getFillColor();
//        // Ánh xạ alphaSpring (0 -> 255) thành độ mờ của nền (0 -> 220)
//        bgColor.a = (sf::Uint8)(tColor.a * (220.0f / 255.0f));
//        background.setFillColor(bgColor);
    }

    void DynamicIsland::draw(sf::RenderWindow& window)
    {
//        // Chặn không vẽ nếu nó đang rảnh và tàng hình 100% để đỡ tốn tài nguyên
//        if (currentState == State::Idle && wSpring.position < 42.0f && alphaSpring.position < 2.0f)
//            return;

        window.draw(background);

        if (alphaSpring.position > 5.0f)
        {
            window.draw(logText);
        }
    }
}
