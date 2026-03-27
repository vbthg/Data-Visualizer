#pragma once
#include <SFML/Graphics.hpp>
#include "Squircle.h"
#include "Spring.h"
#include "DataStructure.h"

namespace GUI
{
    class StructurePanel : public sf::Drawable
    {
    private:
        GUI::Squircle* m_frame;

        // --- CẤU HÀN HÌNH ẢNH & CAMERA ---
        sf::RenderTexture m_contentBuffer; // Tấm bạt ảo để vẽ CTDL
        sf::View m_view;                   // Camera nội bộ của Panel
        float m_currentZoom = 1.0f;

        mutable sf::Shader m_maskShader;
        bool m_isMaskLoaded;

        Utils::Physics::Spring m_widthSpring;
        Utils::Physics::Spring m_heightSpring;
        Utils::Physics::Spring m_alphaSpring;

        Utils::Physics::Spring2D m_viewCenterSpring;

        float m_padding;
        float m_codePanelWidth;
        float m_dockSpace;

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;


        sf::Vector2i m_lastMousePos;
        bool m_isPanning;
        float m_panSpeed;

        Utils::Physics::Spring m_zoomSpring;
        sf::Vector2f m_targetMouseWorld; // Điểm neo để zoom vào

    public:
        StructurePanel();
        ~StructurePanel();

        void initIntro(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen);
        void updateLayout(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen);
        void update(float dt, sf::RenderWindow& window);
        void handleEvent(const sf::Event& event, sf::RenderWindow& window);

        // --- HÀM QUAN TRỌNG CHO PAN/ZOOM ---

        // Vẽ CTDL vào buffer nội bộ
        void renderContent(DataStructure* ds);

        // Chuyển đổi tọa độ chuột từ màn hình vào không gian của CTDL
        sf::Vector2f mapPixelToWorld(sf::Vector2i mousePos, const sf::RenderWindow& window) const;

        void zoomAt(float delta, sf::Vector2f mouseWorldPos);
        void pan(sf::Vector2f offset);

        sf::FloatRect getGlobalBounds() const;
        sf::View& getView() { return m_view; }
    };
}
