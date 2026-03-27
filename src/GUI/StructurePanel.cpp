#include "StructurePanel.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "ViewHandler.h"
#include <iostream>
#include <cmath>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    // Sửa lại đoạn shader trong maskShaderCode
    // Sửa lại đoạn shader trong maskShaderCode
    const std::string maskShaderCode = R"(
        #version 120
        uniform sampler2D u_texture;
        uniform vec2 u_panelSize;
        uniform float u_radius;
        uniform float u_power;

        float sdSuperellipse(vec2 p, vec2 b, float r, float power) {
            vec2 q = abs(p) - b + r;
            vec2 maxQ = max(q, 0.0);
            float distN = pow(pow(maxQ.x, power) + pow(maxQ.y, power), 1.0 / power);
            return min(max(q.x, q.y), 0.0) + distN - r;
        }

        void main() {
            vec2 uv = gl_TexCoord[0].xy;
            vec4 color = texture2D(u_texture, uv);

            vec2 localPixelPos = uv * u_panelSize;
            vec2 halfSize = u_panelSize * 0.5;
            vec2 p = localPixelPos - halfSize;
            float rad = min(u_radius, min(halfSize.x, halfSize.y));
            float dist = sdSuperellipse(p, halfSize, rad, u_power);
            float alphaMask = 1.0 - smoothstep(-0.5, 0.5, dist);

            gl_FragColor = vec4(color.rgb, color.a * alphaMask);
        }
    )";

    StructurePanel::StructurePanel()
    {
        m_isPanning = false;
        m_padding = 24.0f;
        m_codePanelWidth = 450.0f;
        m_dockSpace = 150.0f;

        m_frame = new GUI::Squircle({600.f, 600.f});
        m_frame->setRadius(30.f);
        m_frame->setOutlineThickness(1.5f);
        m_frame->setBakedGlass(&ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 50.png"), sf::Vector2f(1920.f, 1080.f));
        m_frame->setPosition(m_padding, m_padding);
        m_frame->setFillColor(sf::Color::White, 0.3f);
        m_frame->setShadow(Theme::Color::DockShadow, 55.f, {0.f, 20.f});

        m_widthSpring.stiffness = 400.f; m_widthSpring.damping = 40.f;
        m_heightSpring.stiffness = 400.f; m_heightSpring.damping = 40.f;
        m_alphaSpring.stiffness = 300.f; m_alphaSpring.damping = 35.f;

        m_zoomSpring.stiffness = 300.0f; // Thấp hơn một chút để tạo cảm giác "mềm"
        m_zoomSpring.damping = 30.0f;
        m_zoomSpring.snapTo(1.0f); // Mặc định là zoom 1:1

        m_viewCenterSpring.stiffness = 450.0f; // Thấp để trượt êm
        m_viewCenterSpring.damping = 60.0f;    // Ma sát vừa phải để có độ trượt dài
        m_viewCenterSpring.snapTo({960.f, 540.f});

        m_view.setSize(1920.f, 1080.f);
        m_view.setCenter(0.0f, 450.0f);
        m_viewCenterSpring.snapTo(sf::Vector2f(0.f, 450.f));

        // Trong StructurePanel constructor
        m_contentBuffer.create(1920, 1080); // Kích thước cố định lớn nhất
        m_contentBuffer.setSmooth(true);
//        m_view.setCenter(1920.f / 2.0f, 1080.f / 2.0f);

        // Load Shader cắt góc riêng biệt
        m_isMaskLoaded = sf::Shader::isAvailable() && m_maskShader.loadFromMemory(maskShaderCode, sf::Shader::Fragment);
    }

    StructurePanel::~StructurePanel()
    {
        if (m_frame) delete m_frame;
    }

    void StructurePanel::initIntro(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen)
    {
        updateLayout(windowWidth, windowHeight, isCodePanelOpen);
        m_widthSpring.position = m_widthSpring.target * 0.8f;
        m_heightSpring.position = m_heightSpring.target * 0.8f;
        m_alphaSpring.position = 0.0f;
    }

    void StructurePanel::updateLayout(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen)
    {
        m_heightSpring.target = windowHeight - (m_padding * 2.0f) - m_dockSpace;
        m_widthSpring.target = isCodePanelOpen ? (windowWidth - m_codePanelWidth - m_padding * 3.0f) : (windowWidth - m_padding * 2.0f);
        m_alphaSpring.target = 1.0f;
    }

    void StructurePanel::update(float dt, sf::RenderWindow& window)
    {
        // Cập nhật lò xo camera
        m_viewCenterSpring.update(dt);
        m_view.setCenter(m_viewCenterSpring.position);

        m_widthSpring.update(dt);
        m_heightSpring.update(dt);
        m_alphaSpring.update(dt);

        // 2. Logic Zoom mượt mà
        float oldZoom = m_zoomSpring.position;
        m_zoomSpring.update(dt);

        if(std::abs(m_zoomSpring.position - oldZoom) > 0.0001f)
        {
            // Lấy vị trí chuột hiện tại (tính theo pixel window)
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::FloatRect bounds = getGlobalBounds();

            if(bounds.contains(window.mapPixelToCoords(mousePos)))
            {
                // Lấy tọa độ world của chuột TRƯỚC khi thay đổi zoom
                sf::Vector2f mouseWorldBefore = Utils::ViewHandler::mapPixelToWorld(mousePos, window, bounds, m_view);

                // Cập nhật kích thước View theo lò xo
                m_view.setSize(1920.0f * m_zoomSpring.position, 1080.0f * m_zoomSpring.position);

                // Lấy tọa độ world của chuột SAU khi thay đổi zoom
                sf::Vector2f mouseWorldAfter = Utils::ViewHandler::mapPixelToWorld(mousePos, window, bounds, m_view);

                // Dịch chuyển View để điểm dưới chuột không bị chạy mất
                m_view.setCenter(m_view.getCenter() + (mouseWorldBefore - mouseWorldAfter));
            }
            else
            {
                // Nếu chuột nằm ngoài panel, chỉ zoom vào tâm
                m_view.setSize(1920.0f * m_zoomSpring.position, 1080.0f * m_zoomSpring.position);
            }
        }



        float w = std::max(1.0f, m_widthSpring.position);
        float h = std::max(1.0f, m_heightSpring.position);
        m_frame->setSize({w, h});

        float alphaValue = std::max(0.0f, std::min(m_alphaSpring.position, 1.0f));
        m_frame->setFillColor(sf::Color::White, 0.3f * alphaValue);
        m_frame->setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(40 * alphaValue)));
//
//        if(std::abs((float)m_contentBuffer.getSize().x - w) > 2.0f
//        || std::abs((float)m_contentBuffer.getSize().y - h) > 2.0f)
//        {
//            m_contentBuffer.create((unsigned int)w, (unsigned int)h);
//            m_contentBuffer.setSmooth(true);
//
//            // QUAN TRỌNG: Cập nhật lại kích thước View để khớp với Buffer mới
//            // Nếu không, vật thể sẽ bị scale sai tỉ lệ
//            m_view.setSize(w, h);
//        }
    }

    void StructurePanel::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        // Bắt đầu Pan khi nhấn Space
        if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
        {
            m_isPanning = true;
            m_lastMousePos = sf::Mouse::getPosition(window);
        }

        // Kết thúc Pan khi nhả Space
        if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space)
        {
            m_isPanning = false;
        }

        // Thực hiện Pan khi chuột di chuyển và đang giữ Space
        if(event.type == sf::Event::MouseMoved && m_isPanning)
        {
//            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
            sf::Vector2i currentMousePos = {event.mouseMove.x, event.mouseMove.y};

            sf::Vector2f delta;
            delta.x = static_cast<float>(m_lastMousePos.x - currentMousePos.x);
            delta.y = static_cast<float>(m_lastMousePos.y - currentMousePos.y);

            // Cộng dồn vào target của lò xo để tạo độ trượt
            m_viewCenterSpring.target += (delta * m_zoomSpring.position);

            m_lastMousePos = currentMousePos;
//            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
//
//            // Tính toán độ lệch (Delta)
//            sf::Vector2f delta;
//            delta.x = static_cast<float>(m_lastMousePos.x - currentMousePos.x);
//            delta.y = static_cast<float>(m_lastMousePos.y - currentMousePos.y);
//
//            // Di chuyển View theo độ lệch (nhân với hệ số Zoom hiện tại để mượt hơn)
//            m_view.move(delta * m_currentZoom);
//
//            m_lastMousePos = currentMousePos;
        }

        if(event.type == sf::Event::MouseWheelScrolled)
        {
            // Vuốt lên (delta > 0) -> Phóng to (giảm hệ số zoom)
            // Vuốt xuống (delta < 0) -> Thu nhỏ (tăng hệ số zoom)
            float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.15f;

            m_zoomSpring.target *= factor;

            // Giới hạn zoom từ 0.2x đến 5.0x để tránh lỗi hiển thị
            m_zoomSpring.target = std::max(0.4f, std::min(m_zoomSpring.target, 4.0f));
        }
    }

    void StructurePanel::renderContent(DataStructure* ds)
    {
        if (!ds || m_contentBuffer.getSize().x == 0) return;

        m_contentBuffer.clear(sf::Color::Transparent);
        m_contentBuffer.setView(m_view);
        ds->draw(m_contentBuffer);
        m_contentBuffer.display();
    }

    void StructurePanel::zoomAt(float delta, sf::Vector2f mouseWorldPos)
    {
        float zoomFactor = (delta > 0) ? 0.9f : 1.1f;
        sf::Vector2f before = mouseWorldPos;
        m_view.zoom(zoomFactor);
        m_currentZoom *= zoomFactor;
        m_view.setCenter(m_view.getCenter() + (before - m_view.getCenter()) * (1.0f - zoomFactor));
    }

    void StructurePanel::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // 1. Vẽ nền kính mờ (giữ nguyên)
        target.draw(*m_frame, states);

        float w = m_frame->getSize().x;
        float h = m_frame->getSize().y;

        if(w > 0 && h > 0)
        {
            sf::Sprite contentSprite(m_contentBuffer.getTexture());

            // SỬA TẠI ĐÂY: Chỉ lấy phần texture tương ứng với kích thước Panel hiện tại
            // Giống hệt cách bạn làm với Glass Texture trong Squircle.cpp
            contentSprite.setTextureRect(sf::IntRect(0, 0, (int)w, (int)h));

            contentSprite.setPosition(m_frame->getPosition());

            if(m_isMaskLoaded)
            {
                m_maskShader.setUniform("u_texture", sf::Shader::CurrentTexture);
                m_maskShader.setUniform("u_panelSize", sf::Vector2f(w, h));
                m_maskShader.setUniform("u_radius", 30.0f);
                m_maskShader.setUniform("u_power", 4.0f);
                states.shader = &m_maskShader;
            }

            target.draw(contentSprite, states);
        }
    }

    sf::FloatRect StructurePanel::getGlobalBounds() const
    {
        if (m_frame)
        {
            return m_frame->getGlobalBounds();
        }
        return sf::FloatRect();
    }
}
