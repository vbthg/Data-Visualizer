#include "Notch.h"
#include "Easing.h"
#include <iostream>
#include <cmath>
#include <memory>


namespace GUI
{
    sf::Shader Notch::s_shader;
    bool Notch::s_isShaderLoaded = false;

    Notch::Notch(sf::Vector2f size)
        : m_size(size), m_radiusBottom(15.f), m_radiusTopFlare(15.f),
          m_fillColor(sf::Color::Black), m_fillAlpha(1.0f),
          m_shadowColor(sf::Color::Transparent), m_shadowBlur(0.f), m_shadowOffset({0.f, 0.f})
    {
        // Khởi tạo vị trí ban đầu cho Spring để tránh hiện tượng giật kích thước khung hình đầu
        m_sizeSpring.stiffness = 350.f;
        m_sizeSpring.damping = 30.f;
        m_sizeSpring.snapTo(size);

        m_radiusBottomSpring.stiffness = 350.f;
        m_radiusBottomSpring.damping = 30.f;
        m_radiusBottomSpring.snapTo(15.f);

        m_radiusTopFlareSpring.stiffness = 350.f;
        m_radiusBottomSpring.damping = 30.f;
        m_radiusTopFlareSpring.snapTo(23.f);

        // Set stiffness cao và damping vừa phải để có độ nảy "Apple-like"
        m_scaleSpring.snapTo(1.f);
        m_scaleSpring.stiffness = 300.f;
        m_scaleSpring.damping = 20.f;


        if(!s_isShaderLoaded)
        {
            if(sf::Shader::isAvailable())
            {
                if(!s_shader.loadFromMemory(getShaderCode(), sf::Shader::Fragment))
                {
                    std::cerr << "Loi: Khong the nap Notch Shader!\n";
                }
                else
                {
                    s_isShaderLoaded = true;
                }
            }
        }
    }

    void Notch::setSize(sf::Vector2f size)
    {
        // Gán kích thước mục tiêu, Spring trong hàm update() sẽ tự động kéo m_size tới giá trị này
        m_sizeSpring.target = size;
    }

    void Notch::setRadii(float bottomRadius, float topFlareRadius)
    {
        m_radiusBottomSpring.target = bottomRadius;
        m_radiusTopFlareSpring.target = topFlareRadius;
    }

    void Notch::setFillColor(sf::Color color, float alphaTint)
    {
        m_fillColor = color;
        m_fillAlpha = alphaTint;
    }

    void Notch::setShadow(sf::Color color, float blur, sf::Vector2f offset)
    {
        m_shadowColor = color;
        m_shadowBlur = blur;
        m_shadowOffset = offset;
    }

    void Notch::setWaveformState(GUI::Waveform::State state)
    {
        m_waveform.setState(state);
    }

    void Notch::setScaleTarget(float target)
    {
        m_scaleSpring.target = target;
    }

//    void Notch::update(float dt)
//    {
//        // Cập nhật tính toán vật lý lò xo
//        m_sizeSpring.update(dt);
//        // Đồng bộ kích thước hiện tại của Notch với vị trí nội suy của lò xo
//        m_size = m_sizeSpring.position;
//
//        m_radiusBottomSpring.update(dt);
//        m_radiusTopFlareSpring.update(dt);
//        m_radiusBottom = m_radiusBottomSpring.position;
//        m_radiusTopFlare = m_radiusTopFlareSpring.position;
//    }

    sf::Vector2f Notch::getSize() const
    {
        return m_size;
    }

    sf::FloatRect Notch::getLocalBounds() const
    {
        return m_rect.getLocalBounds();
    }

    sf::FloatRect Notch::getGlobalBounds() const
    {
        return getTransform().transformRect(getLocalBounds());
    }

    void Notch::changeContent(std::unique_ptr<NotchContent> nextContent)
    {
//        std::cout << "[NOTCH_CORE] changeContent called! Vua nhan content moi: " << nextContent.get() << std::endl;
//        std::cout << "[NOTCH_CORE] Pointer cu hien tai dang la: " << m_currentContent.get() << std::endl;

        if(m_currentContent)
        {
            m_oldContent = std::move(m_currentContent);
        }

        m_currentContent = std::move(nextContent);
        m_transitionProgress = 0.0f;
        m_isTransitioning = true;

        // Khởi tạo trạng thái ban đầu cho content mới (nằm hơi cao một chút để trượt xuống)
        m_currentContent->setAlpha(0.0f);
        m_currentContent->setVerticalOffset(-15.0f);
    }

    void Notch::update(float dt)
    {
        m_scaleSpring.update(dt);

        // Áp dụng scale vào Transformable (Notch kế thừa từ sf::Transformable)
        float currentScale = m_scaleSpring.position;
        setScale(currentScale, currentScale);
        // Đảm bảo việc scale diễn ra từ tâm của cạnh trên (Origin ở giữa-trên)
        setOrigin(m_size.x / 2.0f, 0.f);


        // 1. Cập nhật vật lý cho khung hình Notch
        m_sizeSpring.update(dt);
        m_radiusBottomSpring.update(dt);
        m_radiusTopFlareSpring.update(dt);

        m_size.x = std::max(10.0f, m_sizeSpring.position.x);
        m_size.y = std::max(10.0f, m_sizeSpring.position.y);
        m_radiusBottom = std::max(0.0f, m_radiusBottomSpring.position);
        m_radiusTopFlare = std::max(0.0f, m_radiusTopFlareSpring.position);

        // --- SỬA LẠI ĐOẠN NÀY ---
        // 1. Dời tâm của chính class Notch (Để NotchManager đặt nó giữa màn hình)
        this->setOrigin(m_size.x / 2.0f, 0.0f);

        // 2. Hình nền đen chạy từ 0 đến size.x
        m_rect.setSize(m_size);
        m_rect.setOrigin(0.f, 0.f);
        m_rect.setPosition(0.f, 0.f);
        // ------------------------

        m_waveform.update(dt);
        m_waveform.setPosition(m_size.x - 35.0f, m_size.y / 2.0f);

        m_progressBar.setSize({m_size.x - 50.f, 2.0f}); // Luôn bám theo chiều rộng của Notch
        m_progressBar.update(dt);

        // 2. Cập nhật Logic Transition (Cross-fade)
        if(m_isTransitioning)
        {
            // Tốc độ transition (khoảng 0.4s để hoàn tất)
            m_transitionProgress += dt * 2.5f;
//            m_transitionProgress += dt * 0.2f;
            if(m_transitionProgress >= 1.0f)
            {
                m_transitionProgress = 1.0f;
                m_isTransitioning = false;
                m_oldContent.reset(); // Giải phóng hoàn toàn content cũ
            }

            float t = m_transitionProgress;
            // Dùng EaseOutQuart của bạn để tạo cảm giác trượt mượt mà
            float ease = Utils::Math::Easing::easeOutExpo(t);

            // Xử lý Content cũ: Mờ dần và trượt xuống dưới
            if(m_oldContent)
            {
                m_oldContent->setAlpha(1.0f - t);
                m_oldContent->setVerticalOffset(ease * 15.0f);
                m_oldContent->update(dt, m_size);
//                m_oldContent->setAlpha(t);
//                m_oldContent->setVerticalOffset(-15.f * (1.0 - ease));
//                m_oldContent->update(dt, m_size);
            }

            // Xử lý Content mới: Hiện dần và trượt từ trên về vị trí 0
            if(m_currentContent)
            {
                m_currentContent->setAlpha(t);
                m_currentContent->setVerticalOffset(-15.0f * (1.0f - ease));
                m_currentContent->update(dt, m_size);
            }
        }
        else if(m_currentContent)
        {
            m_currentContent->update(dt, m_size);
        }
    }

    void Notch::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        if(s_isShaderLoaded)
        {
            sf::Vector2f currentScale = getScale();
            sf::Vector2f actualSize = {m_size.x * currentScale.x, m_size.y * currentScale.y};

            float padding = 0.f;
            if(m_shadowColor.a > 0)
            {
                padding = (m_shadowBlur + std::max(std::abs(m_shadowOffset.x), std::abs(m_shadowOffset.y))) * 2.0f;
            }
            float flarePadding = m_radiusTopFlare * 2.0f * currentScale.x;

            sf::Vector2f totalSize = sf::Vector2f(m_size.x + flarePadding, m_size.y) + sf::Vector2f(padding, padding);
            sf::Vector2f actualTotalSize = {totalSize.x * currentScale.x, totalSize.y * currentScale.y};

            s_shader.setUniform("u_size", actualSize);
            s_shader.setUniform("u_totalSize", actualTotalSize);
            s_shader.setUniform("u_radiusBottom", m_radiusBottom * currentScale.x);
            s_shader.setUniform("u_radiusTopFlare", m_radiusTopFlare * currentScale.x);
            s_shader.setUniform("u_fillColor", sf::Glsl::Vec4(m_fillColor));
            s_shader.setUniform("u_fillAlpha", m_fillAlpha);
            s_shader.setUniform("u_shadowColor", sf::Glsl::Vec4(m_shadowColor));
            s_shader.setUniform("u_shadowBlur", m_shadowBlur * currentScale.x);
            s_shader.setUniform("u_shadowOffset", sf::Vector2f(m_shadowOffset.x * currentScale.x, m_shadowOffset.y * currentScale.y));

            states.shader = &s_shader;

            m_rect.setSize(totalSize);
            m_rect.setOrigin(totalSize / 2.0f);
            m_rect.setPosition(m_size.x / 2.0f, m_size.y / 2.0f);
            m_rect.setTextureRect(sf::IntRect(0, 0, 1, 1));

            target.draw(m_rect, states);

            sf::Shader::bind(NULL);

            // Vẽ ngay trước khi bind(NULL)
            // 3. VẼ PROGRESS BAR Ở DƯỚI ĐÁY (THÊM ĐOẠN NÀY)
//            if(m_currentScenario == GUI::Scenario::Processing)
//            {
//                sf::RenderStates barStates = states;
//                barStates.shader = nullptr;
//                barStates.transform.translate(25.f, m_size.y - m_progressBar.getSize().y - 5.f); // Dịch lên 10.0f để tránh bị che khuất ở mép dưới
//                target.draw(m_progressBar, barStates);
//            }


            // 2. NGẮT SHADER NGAY LẬP TỨC ĐỂ CỨU SFML TEXT
//            sf::Shader::bind(NULL);

            // 2. Trạng thái SẠCH 100% dành cho Chữ và Waveform
            sf::RenderStates cleanStates = states;
            cleanStates.shader = nullptr; // Chốt hạ: Cấm tuyệt đối Shader can thiệp!

            // 3. Vẽ nội dung bằng trạng thái sạch
            if(m_currentScenario != GUI::Scenario::FileTray) target.draw(m_waveform, cleanStates);
//            if(m_oldContent) target.draw(*m_oldContent, cleanStates);
            if(m_currentContent) target.draw(*m_currentContent, cleanStates);

//            target.draw(m_waveform, states); // THÊM DÒNG NÀY VÀO NGAY SAU
//
//            // 2. NGẮT SHADER NGAY LẬP TỨC ĐỂ CỨU SFML TEXT
//            sf::Shader::bind(NULL);
//
//            sf::RenderStates contentStates = states;
//            // states hiện tại đã bao gồm transform của Notch rồi
//            if(m_oldContent) target.draw(*m_oldContent, contentStates);
//            if(m_currentContent) target.draw(*m_currentContent, contentStates);
        }
    }

    std::string Notch::getShaderCode()
    {
        return R"(
            #version 120
            uniform vec2 u_size;
            uniform vec2 u_totalSize;
            uniform float u_radiusBottom;
            uniform float u_radiusTopFlare;
            uniform vec4 u_fillColor;
            uniform float u_fillAlpha;
            uniform vec4 u_shadowColor;
            uniform float u_shadowBlur;
            uniform vec2 u_shadowOffset;

            // Hòa trộn Alpha Blending giúp viền không bị viền đen (pre-multiplied alpha issue)
            vec4 alphaBlend(vec4 top, vec4 bottom)
            {
                float outA = top.a + bottom.a * (1.0 - top.a);
                if(outA == 0.0)
                {
                    return vec4(0.0);
                }
                return vec4((top.rgb * top.a + bottom.rgb * bottom.a * (1.0 - top.a)) / outA, outA);
            }

            // Hàm tính toán khoảng cách SDF cho toàn bộ hình dáng Notch (bao gồm vòm ngược)
            float sdNotch(vec2 p, vec2 b, float rb, float rt)
            {
                vec2 q = vec2(abs(p.x), p.y);
                float rBox = (q.y > 0.0) ? rb : 0.0;
                vec2 qBox = q - b + rBox;
                float dBox = min(max(qBox.x, qBox.y), 0.0) + length(max(qBox, 0.0)) - rBox;

                // Xử lý khoét lõm vòm ngược ở 2 góc trên (Inverse Fillet)
                if(q.x > b.x)
                {
                    vec2 cFlare = vec2(b.x + rt, -b.y + rt);
                    if(q.y < -b.y + rt)
                    {
                        if(q.x < b.x + rt)
                        {
                            dBox = rt - length(q - cFlare);
                            if(q.y < -b.y)
                            {
                                dBox = max(dBox, -b.y - q.y);
                            }
                        }
                        else
                        {
                            dBox = length(q - vec2(b.x + rt, -b.y));
                        }
                    }
                }
                return dBox;
            }

            // Hàm tính toán khoảng cách SDF thuần túy cho thân dưới (Dùng cho Shadow để không lem lên vòm trên)
            float sdShadowBox(vec2 p, vec2 b, float rb)
            {
                vec2 q = vec2(abs(p.x), p.y);
                float rBox = (q.y > 0.0) ? rb : 0.0;
                vec2 qBox = q - b + rBox;
                return min(max(qBox.x, qBox.y), 0.0) + length(max(qBox, 0.0)) - rBox;
            }

            void main()
            {
                vec2 halfSize = u_size * 0.5;
                // Ánh xạ tọa độ màn hình sang tọa độ tâm của Notch
                vec2 p = gl_TexCoord[0].xy * u_totalSize - u_totalSize * 0.5;

                // Tính toán khoảng cách để vẽ thân Notch chính
                float dist = sdNotch(p, halfSize, u_radiusBottom, u_radiusTopFlare);
                float shapeAlpha = 1.0 - smoothstep(-0.5, 0.5, dist);

                vec4 shadowResult = vec4(0.0);
                if(u_shadowColor.a > 0.0)
                {
                    vec2 shadowP = p - u_shadowOffset;
                    // Bóng đổ chỉ áp dụng cho nửa dưới của Notch
                    float sDist = sdShadowBox(shadowP, halfSize, u_radiusBottom);
                    float shadowAlphaVal = exp(-5.0 * pow(max(sDist + u_shadowBlur * 0.5, 0.0) / max(u_shadowBlur, 0.001), 2.0));

                    // Cắt bỏ phần bóng đổ lọt vào trong thân Notch
                    shadowAlphaVal *= (1.0 - shapeAlpha);
                    shadowResult = vec4(u_shadowColor.rgb, u_shadowColor.a * shadowAlphaVal);
                }

                vec4 baseColor = vec4(u_fillColor.rgb, u_fillAlpha);
                baseColor.a *= shapeAlpha;

                // Trộn màu thân Notch lên trên bóng đổ
                gl_FragColor = alphaBlend(baseColor, shadowResult);
            }
        )";
    }


    void Notch::setStepInfo(int current, int total, float duration)
    {
        m_progressBar.setStep(current, total, duration);
    }
}
