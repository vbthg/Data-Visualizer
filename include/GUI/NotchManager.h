#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "GUI/Notch.h"
#include "NotchEnums.h"
#include "WindowConfig.h"

namespace GUI
{

    class NotchManager : public sf::Drawable
    {
    public:
        // Singleton Instance
        static NotchManager& getInstance()
        {
            static NotchManager instance;
            return instance;
        }

        // Xóa các hàm copy/assign để đảm bảo Singleton độc nhất
        NotchManager(const NotchManager&) = delete;
        NotchManager& operator=(const NotchManager&) = delete;

        // Khởi tạo Notch và truyền kích thước màn hình để căn giữa
        void init(sf::Vector2f screenSize = sf::Vector2f(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT));

        // API Duy nhất để các Cấu trúc dữ liệu (AVL, Graph...) giao tiếp với UI
        // Ví dụ: pushNotification(Scenario::Processing, "Comparing", "15 > 10", "MagnifyingGlass");
        void pushNotification(Scenario type, const sf::String& title = "", const sf::String& subtitle = "", const sf::String& iconCode = "");

        void update(float dt);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void updateMousePos(sf::Vector2i mousePos, const sf::RenderWindow& window);
        sf::FloatRect getBounds() const;


        void updateStep(int current, int total, float duration)
        {
            if(m_notch)
            {
                m_notch->setScenario(GUI::Scenario::Processing);
                m_notch->setStepInfo(current, total, duration);
            }
        }

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        NotchManager();
        ~NotchManager() = default;

        // Hàm nội bộ để kích hoạt lò xo đổi kích thước
        void applySize(NotchSize targetSize);

    private:
        std::unique_ptr<GUI::Notch> m_notch;

        Scenario m_currentScenario;
        NotchSize m_currentSizeState;
        sf::Vector2f m_screenSize;

        // Hệ thống tự động thu hồi (Auto-dismiss)
        float m_dismissTimer;
        float m_dismissTargetTime;
        bool m_isAutoDismissing;

        // Bộ thông số chuẩn Apple (Kích thước: x = width, y = height)
        const sf::Vector2f SIZE_COMPACT = {220.f, 40.f};
        const sf::Vector2f SIZE_STANDARD = {400.f, 50.f};
        const sf::Vector2f SIZE_EXPANDED = {500.f, 80.f};
        const sf::Vector2f SIZE_TRAY = {720.f, 170.f};

        // Bán kính bo góc (x = bottom, y = top flare)
        const sf::Vector2f RADII_COMPACT = {14.f, 7.f};
        const sf::Vector2f RADII_STANDARD = {14.f, 10.f};
        const sf::Vector2f RADII_EXPANDED = {14.f, 12.f};
        const sf::Vector2f RADII_TRAY = {38.f, 15.f};
    };
}
