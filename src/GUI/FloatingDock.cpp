#include "FloatingDock.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "Smoothing.h"
#include "drawGlassPane.h"
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
    }

    FloatingDock::~FloatingDock()
    {
        clearItems();
    }

    void FloatingDock::clearItems()
    {
        for(auto item : m_items) delete item;
        m_items.clear();
    }

    void FloatingDock::addItem(DockItem* item)
    {
        m_items.push_back(item);
        updateLayout();
    }

    void FloatingDock::setCommands(const std::vector<DS::Command>& cmds, const sf::Font& iconFont)
    {
        clearItems();

        for(const auto& cmd : cmds)
        {
            auto btn = new Button(iconFont, cmd.name, {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
            btn->applyPreset(ButtonPreset::Ghost);
            btn->setCornerRadius(Theme::Style::DockHighlightRadius);

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
        }

        updateLayout();
    }

    void FloatingDock::setBlurTexture(const sf::Texture& texture)
    {
        m_blurTexture = &texture;
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
        auto& res = ResourceManager::getInstance();
        std::string shadowName = "assets/textures/shadow_" + std::to_string(std::min((int)m_items.size(), 7)) + ".png";
        shadowSprite.setTexture(res.getTexture(shadowName));
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

    void FloatingDock::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        bool isHoveringAny = false;

        for(auto item : m_items)
        {
            item->handleEvent(event, window);

            if(item->isHovering())
            {
                isHoveringAny = true;
                targetHighlightAlpha = Theme::Color::HighlightActiveAlpha;

                // Highlighter trượt mượt mà bám theo Item đang hover
                highlightX.target = item->getPosition().x - position.x;
                highlightWidth.target = item->getWidth(); // Highlighter tự co dãn theo component
            }
        }

        if(!isHoveringAny)
        {
            targetHighlightAlpha = Theme::Color::HighlightIdleAlpha;
            highlightWidth.target = Theme::Style::HighlightIdleWidth;
        }
    }

    void FloatingDock::update(float dt, sf::RenderWindow& window)
    {
        // 1. Cập nhật vật lý cho Dock
        ySpring.update(dt);
        position.y = ySpring.position;

        // QUAN TRỌNG: Cập nhật lò xo bề ngang!
        // Nếu SpeedController nở ra, m_items sẽ báo getWidth() to hơn,
        // updateLayout() thay đổi target, và dòng này sẽ làm bề ngang Dock nở ra mượt mà.
        widthSpring.update(dt);
        float currentDockWidth = widthSpring.position;

        // 2. Cập nhật nền Squircle
        background.setSize({currentDockWidth, dockHeight});
        background.setOrigin(currentDockWidth / 2.0f, 0.0f);
        background.setPosition(position);

        shadowSprite.setPosition(position.x, position.y + dockHeight / 2.0f);

        // 3. Dàn trải lại vị trí các component dựa trên bề ngang đang co dãn
        repositionChildren(currentDockWidth);

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
        window.draw(shadowSprite);

        // Dùng m_blurTexture nếu đã được set
        if (m_blurTexture)
        {
            Utils::Graphics::drawGlassPane(window, background,
                                          *m_blurTexture,
                                          sf::Color(255, 255, 255, 20));
        }
        else
        {
            // Fallback an toàn nếu lỡ quên set
            window.draw(background);
        }

        if(currentHighlightAlpha > 1.0f)
        {
            window.draw(highlighter);
        }

        for(auto item : m_items)
        {
            item->draw(window);
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
