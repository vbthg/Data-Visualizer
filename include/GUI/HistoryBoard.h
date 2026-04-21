#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "HistoryEntry.h"
#include "TimelineManager.h"
#include "GUI/Squircle.h"
#include "GUI/Scrollbar.h"
#include "Spring.h"

namespace GUI
{
    class HistoryBoard : public sf::Drawable, public sf::Transformable
    {
    private:
        std::vector<Core::HistoryOperation> m_uiRecords;
        GUI::Squircle m_background;
        GUI::Scrollbar m_scrollbar;

        float m_width = 350.f;
        float m_height = 500.f;
        float m_rowHeight = 40.f;
        float m_indentSize = 30.f;
        float m_maxScrollY = 0.f;
        float m_totalCalculatedHeight = 0.f; // Fix lỗi 168

        bool m_wasLastFrameHand = false;
        bool m_isOpen = false;
        float m_animationProgress = 0.f;
        const float m_animationSpeed = 2.0f;
        Utils::Physics::Spring m_scrollSpring;
        Utils::Physics::Spring m_scaleSpring;
        Utils::Physics::Spring2D m_boardSpring;

        int m_activeSnapshotIdx = -1; // Fix lỗi 88
        int m_hoveredRowIdx = -1;     // Fix lỗi 132
        std::function<void(int)> m_onJumpCallback = nullptr;

        sf::Vector2f m_anchorPos; // Vị trí "nhà" của Board khi mở hoàn toàn

    public:
        HistoryBoard();
        ~HistoryBoard() = default;

        // Đổi Core::HistoryManager thành Core::TimelineManager
        void syncWithManager(const Core::TimelineManager& manager);
        void toggle();
        bool isOpen() const { return m_isOpen; }

        int handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void update(float dt, sf::RenderWindow& window, int currentManagerIdx);
        void autoScrollToActive(); // Fix lỗi 91 và 172

        void setOnJumpCallback(std::function<void(int)> callback) { m_onJumpCallback = callback; }

        // Ghi đè hàm setPosition của SFML
        void setPosition(float x, float y);
        void setPosition(const sf::Vector2f& pos);

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        // Fix lỗi 370: Thêm isActive vào tham số cuối
        void drawRow(sf::RenderTarget& target, sf::RenderStates states,
                     const std::string& textStr, const sf::String& iconUnicode,
                     float yPos, bool isHeader, bool isActive, sf::Uint8 alpha, float rotation) const;

        void drawHoverEffect(sf::RenderTarget& target, float yPos) const; // Fix lỗi 284
        void drawActiveEffect(sf::RenderTarget& target, float yPos, bool isActive) const; // Fix lỗi 296
        void updateScrollPhysics(float dt);
        void updateHoverAndCursor(sf::RenderWindow& window);
        void updateOpenAnimation(float dt);
        void updateTotalHeight();
//        void updateOpenAnimation(float dt);
        sf::Vector2f getMouseInFrame(sf::RenderWindow& window);

    };
}
