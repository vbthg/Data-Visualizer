#pragma once
#include <SFML/Graphics.hpp>
#include "Squircle.h"
#include "Spring.h"
#include "DataStructure.h"
#include "TimelineManager.h"
#include "ResourceManager.h"
#include "NodeUI.h"
#include "EdgeUI.h"
#include "Button.h"

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

        sf::Vector2f m_mouseWorldPos; // Lưu tọa độ thật của chuột trong không gian 1920x1080

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

        sf::Vector2f mapPanelPixelToWorld(sf::Vector2i mousePos, const sf::RenderWindow& window, float zoom = -1.f);


        sf::Vector2i m_lastMousePos;
        bool m_isPanning;
        float m_panSpeed;

        Utils::Physics::Spring m_zoomSpring;
        sf::Vector2f m_targetMouseWorld; // Điểm neo để zoom vào

        // Quản lý bộ nhớ của các Node và Edge đang hiển thị
        std::unordered_map<int, std::unique_ptr<NodeUI>> m_nodeUIMap;

        // Key của Edge có thể là string ghép từ 2 ID: "startId-endId"
        std::unordered_map<uint64_t, std::unique_ptr<EdgeUI>> m_edgeUIMap;

        // Hàm phụ trợ giúp ghép 2 ID thành 1 Key duy nhất
        static uint64_t getEdgeKey(int startId, int endId)
        {
            // Đẩy startId lên 32 bit cao, giữ endId ở 32 bit thấp
            return (static_cast<uint64_t>(startId) << 32) | static_cast<uint32_t>(endId);
        }


        sf::FloatRect calculateBoundingBox();

        void fitView(bool immediate);

        bool m_needsReposition = false;
        int m_lastNodeCount = 0;

        bool m_isAutoFollow = true;

        sf::Text m_emptyMessage;
//        float m_autoFitTimer = 0.f;

        GUI::Button* resetBtn;



    public:
        StructurePanel();
        ~StructurePanel();

        void initIntro(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen);
        void updateLayout(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen);
        void update(float dt, sf::RenderWindow& window);
        void handleEvent(const sf::Event& event, sf::RenderWindow& window);

        // Hàm phụ trợ để đồng bộ dữ liệu
        void syncGraphObjects(const Core::RenderFrame& frame, float dt);

        // --- HÀM QUAN TRỌNG CHO PAN/ZOOM ---

        // Vẽ CTDL vào buffer nội bộ
        void renderContent();

        // Chuyển đổi tọa độ chuột từ màn hình vào không gian của CTDL
        sf::Vector2f mapPixelToWorld(sf::Vector2i mousePos, const sf::RenderWindow& window) const;

        void zoomAt(float delta, sf::Vector2f mouseWorldPos);
        void pan(sf::Vector2f offset);

        sf::FloatRect getGlobalBounds() const;
        sf::View& getView() { return m_view; }
    };
}
