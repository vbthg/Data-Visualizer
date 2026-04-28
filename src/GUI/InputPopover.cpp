#include "InputPopover.h"
#include "Theme.h"
#include <iostream>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    // Giá trị khởi tạo giả để tránh lỗi con trỏ
    DS::Command emptyCmd("Empty", "Empty", DS::InputType::None, [](DS::InputArgs){});

    InputPopover::InputPopover(const sf::Font& font)
        : currentState(State::Closed), currentCommand(emptyCmd)
    {
        // 1. Nền Popover
        background.setFillColor(Theme::Color::Surface);
        // Tạm ẩn
        background.setSize({0.f, 0.f});

        // 2. Tiêu đề
        titleText.setFont(font);
        titleText.setCharacterSize(22);
        titleText.setFillColor(Theme::Color::TextPrimary);

        // 3. Ô nhập liệu (Sử dụng InputField vừa thiết kế)
        input1 = new InputField(font, InputField::Type::Integer, {200.f, 44.f});
        input2 = new InputField(font, InputField::Type::Integer, {200.f, 44.f});

        // 4. Nút bấm
        btnConfirm = new Button(font, "Confirm", {120.f, 40.f});
        btnConfirm->applyPreset(ButtonPreset::Primary);

        btnCancel = new Button(font, "Cancel", {100.f, 40.f});
        btnCancel->applyPreset(ButtonPreset::Ghost);

        // Gán sự kiện nút
        btnConfirm->onClick = [this]() { this->submitData(); };
        btnCancel->onClick = [this]() { this->close(); };

        // 5. Setup Lò xo Morphing (Cấu hình "Bouncy" mượt mà)
        float stiff = 200.0f;
        float damp = 20.0f;

        xSpring.stiffness = stiff; xSpring.damping = damp;
        ySpring.stiffness = stiff; ySpring.damping = damp;
        wSpring.stiffness = stiff; wSpring.damping = damp;
        hSpring.stiffness = stiff; hSpring.damping = damp;
        rSpring.stiffness = stiff; rSpring.damping = damp;

        // Alpha cần nhanh hơn một chút để chữ không bị nhòe lúc hộp còn nhỏ
        alphaSpring.stiffness = 300.0f; alphaSpring.damping = 25.0f;

        alphaSpring.snapTo(0.0f);
    }

    InputPopover::~InputPopover()
    {
        delete input1;
        delete input2;
        delete btnConfirm;
        delete btnCancel;
    }

    void InputPopover::open(sf::Vector2f startPos, const DS::Command& cmd)
    {
        if (currentState != State::Closed) return;

        currentCommand = cmd;
        originPos = startPos;
        currentState = State::Opening;

        // Reset dữ liệu cũ
        input1->clear();
        input2->clear();

        titleText.setString(cmd.name);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);

        // Thiết lập loại Input
        if (cmd.inputType == DS::InputType::String)
        {
            // Nếu không có hàm đổi Type, bạn có thể tạo hàm setType trong InputField sau
            // Tạm thời bỏ qua, mặc định input1 đang là Integer. Bạn nhớ cập nhật hàm đó nhé!
            input1->setPlaceholder("Enter text...");
        }
        else if (cmd.inputType == DS::InputType::Integer)
        {
            input1->setPlaceholder("Enter a number...");
        }
        else if (cmd.inputType == DS::InputType::TwoIntegers)
        {
            input1->setPlaceholder("Index...");
            input2->setPlaceholder("Value...");
        }

        // --- 1. Ép vị trí ban đầu (Snap) về bằng đúng cái nút ---
        xSpring.snapTo(startPos.x);
        ySpring.snapTo(startPos.y);
        wSpring.snapTo(44.0f); // Kích thước của nút Dock
        hSpring.snapTo(44.0f);
        rSpring.snapTo(16.0f); // Bo góc của nút Dock
        alphaSpring.snapTo(0.0f);

        // --- 2. Đặt mục tiêu bung nở (Target) ---
        // Giả sử bung ra kích thước chuẩn là 300x180
        float targetW = 320.0f;
        float targetH = (cmd.inputType == DS::InputType::TwoIntegers) ? 220.0f : 180.0f;

        // Bay lên cách nút Dock 150px
        xSpring.target = startPos.x;
        ySpring.target = startPos.y - 150.0f;
        wSpring.target = targetW;
        hSpring.target = targetH;
        rSpring.target = 24.0f; // Bo góc lớn hơn cho bảng
        alphaSpring.target = 255.0f; // Hiện chữ lên

        // Auto focus vào ô đầu tiên
        input1->setFocus(true);
    }

    void InputPopover::close()
    {
        currentState = State::Closing;

        // Mờ chữ đi
        alphaSpring.target = 0.0f;

        // Lò xo kéo ngược về vị trí nút Dock
        xSpring.target = originPos.x;
        ySpring.target = originPos.y;
        wSpring.target = 44.0f;
        hSpring.target = 44.0f;
        rSpring.target = 16.0f;
    }

    void InputPopover::setInputType(DS::InputType inputType)
    {
        if(inputType == DS::InputType::Integer || inputType == DS::InputType::TwoIntegers)
        {
            input1->setType(GUI::InputField::Type::Integer);
            input2->setType(GUI::InputField::Type::Integer);
        }
        else
        {
            input1->setType(GUI::InputField::Type::String);
            input2->setType(GUI::InputField::Type::String);
        }
    }

    void InputPopover::submitData()
    {
        // Kiểm tra dữ liệu rỗng
        std::string s1 = input1->getText();
        if (s1.empty()) return; // [TODO: Thể hiện rung lắc báo lỗi ở đây nếu muốn]

        DS::InputArgs args;

        try
        {
            if (currentCommand.inputType == DS::InputType::Integer)
            {
                args.iVal1 = std::stoi(s1);
            }
            else if (currentCommand.inputType == DS::InputType::String)
            {
                args.sVal = s1;
            }
            else if (currentCommand.inputType == DS::InputType::TwoIntegers)
            {
                std::string s2 = input2->getText();
                if (s2.empty()) return;

                args.iVal1 = std::stoi(s1);
                args.iVal2 = std::stoi(s2);
            }

            // Gửi dữ liệu xuống thuật toán
            currentCommand.action(args);

            // Xong việc thì đóng bảng
            close();
        }
        catch (...)
        {
            std::cout << "Parse error!" << std::endl;
        }
    }

    bool InputPopover::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        // Nếu không mở thì không nhận bất cứ sự kiện nào
        if (currentState != State::Open && currentState != State::Opening) return false;

        // Bắt sự kiện phím Enter
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
        {
            submitData();
            return true; // Đã xử lý, không cho phím Enter trôi xuống dưới
        }

        // Bắt sự kiện ESC
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            close();
            return true;
        }

        bool handled = false;

        // Chuyển sự kiện cho các input và button con
        // Nếu bất kỳ thằng con nào nhận (handled = true), Popover cũng báo là đã nhận
        if (input1->handleEvent(event, window)) handled = true;

        if (currentCommand.inputType == DS::InputType::TwoIntegers)
        {
            if (input2->handleEvent(event, window)) handled = true;
        }

        if (btnConfirm->handleEvent(event, window)) handled = true;
        if (btnCancel->handleEvent(event, window)) handled = true;

        return handled;
    }

    void InputPopover::updateLayout()
    {
        float curX = xSpring.position;
        float curY = ySpring.position;

        background.setPosition(curX, curY);
        background.setSize({wSpring.position, hSpring.position});
        background.setOrigin(wSpring.position / 2.0f, hSpring.position / 2.0f);
        background.setRadius(rSpring.position);

        // Cập nhật vị trí các nội dung bên trong dựa theo tâm (curX, curY)
        titleText.setPosition(curX, curY - hSpring.position / 2.0f + 30.0f);

        if (currentCommand.inputType == DS::InputType::TwoIntegers)
        {
            input1->setPosition({curX, curY - 20.0f});
            input2->setPosition({curX, curY + 30.0f});
            btnCancel->setPosition({curX - 60.0f, curY + 80.0f});
            btnConfirm->setPosition({curX + 60.0f, curY + 80.0f});
        }
        else
        {
            input1->setPosition({curX, curY});
            btnCancel->setPosition({curX - 60.0f, curY + 50.0f});
            btnConfirm->setPosition({curX + 60.0f, curY + 50.0f});
        }

        // Áp dụng độ mờ (Alpha) cho nội dung
        sf::Uint8 alpha = (sf::Uint8)std::max(0.0f, std::min(alphaSpring.position, 255.0f));

        // Set Alpha cho Title
        sf::Color tColor = titleText.getFillColor();
        tColor.a = alpha;
        titleText.setFillColor(tColor);

        // (Do Button và InputField chưa có hàm setOpacity toàn cục trong code hiện tại của bạn,
        // nếu bạn muốn các nút và input cũng mờ dần, bạn phải gọi hàm setOpacity cho chúng.
        // Tạm thời chúng ta sẽ dùng kỹ thuật "Chỉ vẽ nội dung khi Alpha > x" ở hàm draw).
    }

    void InputPopover::update(float dt, sf::RenderWindow& window)
    {
        if (currentState == State::Closed) return;

        // 1. Cập nhật Lò xo
        xSpring.update(dt);
        ySpring.update(dt);
        wSpring.update(dt);
        hSpring.update(dt);
        rSpring.update(dt);
        alphaSpring.update(dt);

        // 2. Chuyển đổi trạng thái
        // Nếu đang mở và lò xo alpha đạt đỉnh -> Mở hoàn tất
        if (currentState == State::Opening && alphaSpring.position > 250.0f)
        {
            currentState = State::Open;
        }
        // Nếu đang đóng và hộp thu nhỏ lại gần bằng cái nút -> Đóng hoàn tất
        if (currentState == State::Closing && wSpring.position < 50.0f)
        {
            currentState = State::Closed;
        }

        // 3. Cập nhật Layout
        updateLayout();

        // 4. Update component logic
        input1->update(dt);
        if (currentCommand.inputType == DS::InputType::TwoIntegers) input2->update(dt);

        btnConfirm->update(window, dt); // Sửa lại tùy signature hàm update của bạn
        btnCancel->update(window, dt);
    }

    void InputPopover::draw(sf::RenderWindow& window)
    {
        if (currentState == State::Closed) return;

        // 1. Vẽ cái nền Morphing (Lúc nào cũng vẽ)
        window.draw(background);

        // 2. Chỉ vẽ nội dung khi bảng đã nở ra một chút (tránh chữ bị tràn viền lúc nhỏ)
        if (alphaSpring.position > 50.0f)
        {
            window.draw(titleText);

//            input1->draw(window);
            window.draw(*input1);
            if (currentCommand.inputType == DS::InputType::TwoIntegers)
            {
                window.draw(*input2);
            }

            btnConfirm->draw(window);
            btnCancel->draw(window);
        }
    }
}
