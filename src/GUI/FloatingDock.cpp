#include "FloatingDock.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "Smoothing.h"
//#include "drawGlassPane.h"
#include <iostream>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    FloatingDock::FloatingDock()
    {
        dockHeight = Theme::Style::DockHeight;
        position = {0.0f, 0.0f};

        background.setRadius(Theme::Style::DockRadius);
        background.setOutlineThickness(0.5f);
        background.setOutlineColor(sf::Color(255, 255, 255, 40)); // Viền kính trắng mờ
        background.setFillColor(sf::Color::White, 0.1f);
        background.setShadow(Theme::Color::DockShadow, 55.f, {0.f, 20.f});
        // Không setFillColor nữa vì ta sẽ dùng Glass Pane

        highlighter.setRadius(Theme::Style::DockHighlightRadius);
        highlighter.setFillColor(sf::Color(0, 0, 0, 0));

        highlightX.stiffness = Theme::Animation::HighlightStiffness;
        highlightX.damping = Theme::Animation::HighlightDamping;
        highlightWidth.stiffness = Theme::Animation::HighlightStiffness;
        highlightWidth.damping = Theme::Animation::HighlightDamping;

        currentHighlightAlpha = Theme::Color::HighlightIdleAlpha;
        targetHighlightAlpha = Theme::Color::HighlightIdleAlpha;

        ySpring.stiffness = Theme::Animation::SlideStiffness;
        ySpring.damping = Theme::Animation::SlideDamping;

        // Cấu hình lò xo chiều rộng (Đầm & Mượt)
        widthSpring.stiffness = 400.0f;
        widthSpring.damping = 30.0f;
        widthSpring.position = 0.0f;
        widthSpring.target = 0.0f;

        // Cấu hình font cho Tooltip một lần ở đây
        m_tooltipText.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProText-regular.ttf"));
        m_tooltipText.setCharacterSize(14);
        m_tooltipText.setFillColor(sf::Color::Black);
    }

    FloatingDock::~FloatingDock()
    {
        clearItems();
    }

    void FloatingDock::clearItems()
    {
        for(auto item : m_items) delete item;
        m_items.clear();
        m_itemNames.clear();
    }

    void FloatingDock::addItem(DockItem* item, std::string name)
    {
        m_items.push_back(item);
        m_itemNames.push_back(name);
        updateLayout();
    }

    void FloatingDock::setCommands(const std::vector<DS::Command>& cmds, const sf::Font& iconFont)
    {
//        clearItems();

        for(const auto& cmd : cmds)
        {
            auto btn = new Button(iconFont, cmd.iconCode, {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
            btn->applyPreset(ButtonPreset::Ghost);
            btn->setCornerRadius(Theme::Style::DockHighlightRadius);
            btn->setCharacterSize(25);

            btn->onClick = [this, btn, cmd]()
            {
                if(cmd.inputType == DS::InputType::None)
                {
                    cmd.action(DS::InputArgs());
                }
                else
                {
                    if(this->onCommandClicked)
                    {
                        this->onCommandClicked(btn->getPosition(), cmd);
                    }
                }
            };

            m_items.push_back(btn);
            m_itemNames.push_back(cmd.name); // Lưu tên vào list riêng để tra cứu
        }

        updateLayout();
    }

    void FloatingDock::setBlurTexture(const sf::Texture& texture, const sf::Vector2f size)
    {
//        m_blurTexture = &texture;
        background.setBakedGlass(&texture, size);
    }

    void FloatingDock::updateLayout()
    {
        if(m_items.empty()) return;

        // Tính toán Target Width (cộng dồn chiều rộng của MỌI component + khoảng cách)
        float totalWidth = Theme::Style::DockPadding; // Lề trái (Padding)

        for(auto item : m_items)
        {
            totalWidth += item->getWidth();
            totalWidth += Theme::Style::DockItemGap; // Gap giữa các items
        }

        totalWidth += Theme::Style::DockPadding - Theme::Style::DockItemGap; // Lề phải (Padding)

        // Ra lệnh cho lò xo bề ngang
        widthSpring.target = totalWidth;

        // Cập nhật bóng đổ (Shadow) động theo số lượng items nếu cần
//        auto& res = ResourceManager::getInstance();
//        std::string shadowName = "assets/textures/shadow_" + std::to_string(std::min((int)m_items.size(), 7)) + ".png";
//        shadowSprite.setTexture(res.getTexture(shadowName));
    }

    void FloatingDock::repositionChildren(float currentWidth)
    {
        // Tính toán điểm bắt đầu vẽ (từ mép trái của Dock hiện tại)
        float startX = position.x - (currentWidth / 2.0f) + 16.0f; // 16.0f là lề trái
        float centerY = position.y + dockHeight / 2.0f;

        for(auto item : m_items)
        {
            float itemWidth = item->getWidth();

            // Tâm của component này sẽ nằm ở đâu?
            float itemCenterX = startX + (itemWidth / 2.0f);
            item->setPosition({itemCenterX, centerY});

            // Nhảy sang vị trí của component tiếp theo (+ Gap)
            startX += itemWidth + 8.0f;
        }
    }

    void FloatingDock::setPosition(float x, float y)
    {
        position = {x, y};
    }

    sf::Vector2f FloatingDock::getPosition() const
    {
        return position;
    }

    void FloatingDock::setTargetY(float targetY)
    {
        ySpring.target = targetY;
    }

    void FloatingDock::snapToY(float startY)
    {
        ySpring.snapTo(startY);
        position.y = startY;
    }

    bool FloatingDock::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        bool handled = false;
        bool isHoveringAny = false;

//        int id = -1;
        for(auto item : m_items)
        {
//            ++id;
            // Chuyển sự kiện cho từng item trong Dock
            if(item->handleEvent(event, window))
            {
                handled = true;
//                std::cout << "[BUTTON CLICKED]: " << id << "\n";
            }

//            if(item->isHovering())
//            {
//                isHoveringAny = true;
//                targetHighlightAlpha = Theme::Color::HighlightActiveAlpha;
//
//                highlightX.target = item->getPosition().x - position.x;
//                highlightWidth.target = item->getWidth();
//            }
        }

        if(!isHoveringAny)
        {
            targetHighlightAlpha = Theme::Color::HighlightIdleAlpha;
            highlightWidth.target = Theme::Style::HighlightIdleWidth;
        }

        // Trả về true nếu người dùng tương tác với bất kỳ item nào trên Dock
        return handled;
    }

    void FloatingDock::update(float dt, sf::RenderWindow& window)
    {
        // 1. Cập nhật vật lý cho Dock
        ySpring.update(dt);
        position.y = ySpring.position;


        updateLayout();


        // QUAN TRỌNG: Cập nhật lò xo bề ngang!
        // Nếu SpeedController nở ra, m_items sẽ báo getWidth() to hơn,
        // updateLayout() thay đổi target, và dòng này sẽ làm bề ngang Dock nở ra mượt mà.
//        widthSpring.update(dt);
        float currentDockWidth = widthSpring.target;

        // 2. Cập nhật nền Squircle
        background.setSize({currentDockWidth, dockHeight});
        background.setOrigin(currentDockWidth / 2.0f, 0.0f);
        background.setPosition(position);

//        shadowSprite.setPosition(position.x, position.y + dockHeight / 2.0f);

        // 3. Dàn trải lại vị trí các component dựa trên bề ngang đang co dãn
        repositionChildren(currentDockWidth);



        // 1. Kiểm tra xem chuột đang hover vào item nào
        int currentHover = -1;
        for(int i = 0; i < (int)m_items.size(); ++i)
        {
            if(m_items[i]->isHovering()) // Giả sử Button của em có hàm này
            {
                currentHover = i;
                break;
            }
        }

        // 2. Cập nhật Timer và Target Alpha
        float targetAlpha = 0.0f;
        if(currentHover != -1)
        {

            if(currentHover == m_hoveredIndex)
            {
                m_hoverTimer += dt;
                if(m_hoverTimer >= HOVER_THRESHOLD)
                {
                    targetAlpha = 255.0f;
                }
            }
            else
            {
                // Nếu vừa chuyển từ nút này sang nút khác, reset timer
                m_hoveredIndex = currentHover;
                m_hoverTimer = 0.0f;
                m_tooltipText.setString(m_itemNames[currentHover]);
            }
        }
        else
        {
            m_hoveredIndex = -1;
            m_hoverTimer = 0.0f;
            targetAlpha = 0.0f;
        }

        // 3. Làm mượt Alpha bằng hàm damp của em
        m_popoverAlpha = Utils::Math::Smoothing::damp(m_popoverAlpha, targetAlpha, 0.001f, dt, 500.0f);

        // 4. Cập nhật vị trí Tooltip nếu nó đang hiển thị
        if(m_popoverAlpha > 1.0f && m_hoveredIndex != -1)
        {
            sf::Vector2f btnPos = m_items[m_hoveredIndex]->getPosition();
            // Căn giữa text theo chiều ngang của Button
            float textWidth = m_tooltipText.getLocalBounds().width;
            m_tooltipText.setOrigin(textWidth / 2.0f, m_tooltipText.getLocalBounds().height);
            m_tooltipText.setPosition(btnPos.x, btnPos.y - Theme::Style::IconButtonSize / 1.5f - 30.0f);
            m_tooltipText.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)m_popoverAlpha));
        }


        // 4. Update các component con
        for(auto item : m_items)
        {
            item->update(window, dt);
        }

        // 5. Update Highlighter
        highlightX.update(dt);
        highlightWidth.update(dt);

        currentHighlightAlpha = Utils::Math::Smoothing::damp(
            currentHighlightAlpha, targetHighlightAlpha, Theme::Animation::AlphaDampSpeed, dt, 150.0f
        );

        highlighter.setSize({highlightWidth.position, Theme::Style::IconButtonSize});
        highlighter.setOrigin(highlightWidth.position / 2.0f, Theme::Style::IconButtonSize / 2.0f);
        highlighter.setPosition(position.x + highlightX.position, position.y + dockHeight / 2.0f);
        highlighter.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)currentHighlightAlpha));
    }

    void FloatingDock::draw(sf::RenderWindow& window)
    {
//        window.draw(shadowSprite);

        // Dùng m_blurTexture nếu đã được set
        window.draw(background);

        if(currentHighlightAlpha > 1.0f)
        {
            window.draw(highlighter);
        }

        for(auto item : m_items)
        {
            item->draw(window);
        }

        if(m_popoverAlpha > 1.0f)
        {
            // Có thể vẽ thêm một cái background nhỏ (rounded rect) cho chữ nếu muốn đẹp hơn
            window.draw(m_tooltipText);
        }
    }

//    void FloatingDock::draw(sf::RenderWindow& window)
//{
//    window.draw(shadowSprite);
//
//    background.setTexture(nullptr);
//    background.setFillColor(sf::Color::Yellow);
//    window.draw(background);
//
//    sf::Vector2f size = background.getSize();
//    std::cout << size.x << " " << size.y << "\n";
//
//    // Dùng đúng đường dẫn bạn đã khai báo bên VisualizerState
//    const sf::Texture& dockBlurTex = ResourceManager::getInstance().getTexture("assets/textures/macOS Sonoma.png");
//
//    // Gọi hàm tiện ích
//    Utils::Graphics::drawGlassPane(window, background, dockBlurTex, sf::Color(255, 0, 0, 80));
//
//    if(currentHighlightAlpha > 1.0f)
//    {
//        window.draw(highlighter);
//    }
//
//    for(auto item : m_items)
//    {
//        item->draw(window);
//    }
//}
}
