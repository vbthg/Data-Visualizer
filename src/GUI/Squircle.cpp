#include "Squircle.h"
#include <iostream>
#include <cmath>

namespace GUI
{
    sf::Shader Squircle::s_shader;
    bool Squircle::s_isShaderLoaded = false;

    const std::string shaderCode = R"(
        #version 120
        uniform vec2 u_size;
        uniform vec2 u_totalSize;
        uniform vec4 u_radii;
        uniform vec4 u_fillColor;
        uniform float u_fillAlpha;

        uniform vec4 u_outlineColor;
        uniform float u_outlineThickness;
        uniform float u_power;

        uniform vec4 u_shadowColor;
        uniform float u_shadowBlur;
        uniform vec2 u_shadowOffset;

        uniform sampler2D u_bakedBlurTexture;
        uniform vec2 u_screenPos;
        uniform vec2 u_resolution;

float sdSuperellipse(vec2 p, vec2 b, float r, float power)
{
    vec2 q = abs(p) - b + r;
    vec2 maxQ = max(q, 0.0);
    float distN;

    if (power > 3.0) // Giả định là 4.0
    {
        float x2 = maxQ.x * maxQ.x;
        float y2 = maxQ.y * maxQ.y;
        distN = sqrt(sqrt(x2 * x2 + y2 * y2));
    }
    else // Giả định là 2.0 (Đường tròn tiêu chuẩn)
    {
        distN = length(maxQ);
    }

    return min(max(q.x, q.y), 0.0) + distN - r;
}

        // Dùng thuật toán Alpha Blending thay vì mix() để hòa trộn màu outline không bị ám đen
        vec4 alphaBlend(vec4 top, vec4 bottom)
        {
            float outA = top.a + bottom.a * (1.0 - top.a);
            if (outA == 0.0)
            {
                return vec4(0.0);
            }
            return vec4((top.rgb * top.a + bottom.rgb * bottom.a * (1.0 - top.a)) / outA, outA);
        }

        void main()
        {
            vec2 halfSize = u_size * 0.5;
            vec2 p = gl_TexCoord[0].xy * u_totalSize - u_totalSize * 0.5;


// 1. Cận an toàn cho bo góc
    float maxRad = max(max(u_radii.x, u_radii.y), max(u_radii.z, u_radii.w));
    float safety = maxRad + u_outlineThickness;

    // 2. Cận an toàn cho Outline (để không bị mất viền ở các cạnh thẳng)
    float edgeSafety = u_outlineThickness + 0.5; // +0.5 để tránh lỗi làm tròn pixel

    // 3. ĐIỀU KIỆN ĐÚNG:
    // Dải đứng: phải hẹp hơn lõi bo góc VÀ phải nằm THỤT VÀO so với biên trên/dưới để chừa chỗ cho viền ngang
    bool inVertical = abs(p.x) < (halfSize.x - safety) && abs(p.y) < (halfSize.y - edgeSafety);

    // Dải ngang: phải thấp hơn lõi bo góc VÀ phải nằm THỤT VÀO so với biên trái/phải để chừa chỗ cho viền dọc
    bool inHorizontal = abs(p.y) < (halfSize.y - safety) && abs(p.x) < (halfSize.x - edgeSafety);

    if (inVertical || inHorizontal)
    {
        // Ở đây ta chắc chắn pixel nằm TRONG vùng fill của Squircle
        // Không có shadow, không có viền, alpha chắc chắn là 1.0

        vec4 baseColor = vec4(u_fillColor.rgb, u_fillAlpha);

        if (u_resolution.x > 0.0)
        {
            vec2 pixelPosOnScreen = u_screenPos + (p + halfSize);
            vec2 screenUV = pixelPosOnScreen / u_resolution;
            vec4 glassColor = texture2D(u_bakedBlurTexture, screenUV);
            gl_FragColor = vec4(mix(glassColor.rgb, u_fillColor.rgb, u_fillAlpha), glassColor.a);
        }
        else
        {
            gl_FragColor = baseColor;
        }
        return;
    }

            float rad = 0.0;
            if (p.x < 0.0 && p.y < 0.0)        rad = u_radii.x;
            else if (p.x >= 0.0 && p.y < 0.0)  rad = u_radii.y;
            else if (p.x >= 0.0 && p.y >= 0.0) rad = u_radii.z;
            else                               rad = u_radii.w;

            rad = min(rad, min(halfSize.x, halfSize.y));

            float dist = sdSuperellipse(p, halfSize, rad, u_power);
            float shapeAlpha = 1.0 - smoothstep(-0.5, 0.5, dist);
            float fillAlpha  = 1.0 - smoothstep(-0.5, 0.5, dist + u_outlineThickness);

            vec4 shadowResult = vec4(0.0);
            if (u_shadowColor.a > 0.0)
            {
                vec2 shadowP = p - u_shadowOffset;
                float sDist = sdSuperellipse(shadowP, halfSize, u_radii.x, u_power);
                float ratio = max(sDist + u_shadowBlur * 0.5, 0.0) / max(u_shadowBlur, 0.001);
float shadowAlphaVal = exp(-5.0 * (ratio * ratio));

                // Cắt bỏ phần bóng đổ lọt vào bên trong shape để giữ đúng màu của kính và outline
                shadowAlphaVal *= (1.0 - shapeAlpha);
                shadowResult = vec4(u_shadowColor.rgb, u_shadowColor.a * shadowAlphaVal);
            }

            vec4 baseColor = vec4(u_fillColor.rgb, u_fillAlpha);

            if (u_resolution.x > 0.0)
            {
                vec2 pixelPosOnScreen = u_screenPos + (p + halfSize);
                vec2 screenUV = pixelPosOnScreen / u_resolution;
                vec4 glassColor = texture2D(u_bakedBlurTexture, screenUV);

                baseColor = vec4(mix(glassColor.rgb, u_fillColor.rgb, u_fillAlpha), glassColor.a);
            }

            vec4 finalColor = mix(u_outlineColor, baseColor, fillAlpha);
            finalColor.a *= shapeAlpha;

            gl_FragColor = alphaBlend(finalColor, shadowResult);
        }
    )";

    Squircle::Squircle(sf::Vector2f size)
        : m_size(size), m_fillColor(sf::Color::White), m_fillAlpha(1.0f), m_bakedTexture(nullptr), m_resolution({0.f, 0.f}),
          m_outlineColor(sf::Color::Transparent), m_outlineThickness(0.f), m_power(4.f),
          m_shadowColor(sf::Color::Transparent), m_shadowBlur(0.f), m_shadowOffset({0.f, 0.f})
    {
        m_rect.setSize(size);
        m_rect.setTextureRect(sf::IntRect(0, 0, 1, 1));

        if (!s_isShaderLoaded)
        {
            if (sf::Shader::isAvailable())
            {
                if (!s_shader.loadFromMemory(shaderCode, sf::Shader::Fragment))
                {
                    std::cerr << "Loi: Khong the nap Glass Shader!\n";
                }
                else
                {
                    s_isShaderLoaded = true;
                }
            }
        }
    }

    void Squircle::setSize(sf::Vector2f size)
    {
        m_size = size;
        m_rect.setSize(size);
    }

    void Squircle::setRadius(float radius)
    {
        m_radii = sf::Glsl::Vec4(radius, radius, radius, radius);
    }

    void Squircle::setRadius(float tl, float tr, float br, float bl)
    {
        m_radii = sf::Glsl::Vec4(tl, tr, br, bl);
    }

    void Squircle::setPower(float power)
    {
        m_power = power;
    }

    void Squircle::setFillColor(sf::Color color, float alphaTint)
    {
        m_fillColor = color;
        m_fillAlpha = alphaTint;
    }

    void Squircle::setOutlineColor(sf::Color color)
    {
        m_outlineColor = color;
    }

    void Squircle::setOutlineThickness(float thickness)
    {
        m_outlineThickness = thickness;
    }

    void Squircle::setShadow(sf::Color color, float blur, sf::Vector2f offset)
    {
        m_shadowColor = color;
        m_shadowBlur = blur;
        m_shadowOffset = offset;
    }

    void Squircle::setBakedGlass(const sf::Texture* bakedTexture, sf::Vector2f windowResolution)
    {
        m_bakedTexture = bakedTexture;
        m_resolution = windowResolution;
    }

    sf::Vector2f Squircle::getSize() const
    {
        return m_size;
    }

    sf::FloatRect Squircle::getLocalBounds() const
    {
        return m_rect.getLocalBounds();
    }

    sf::FloatRect Squircle::getGlobalBounds() const
    {
        return getTransform().transformRect(getLocalBounds());
    }

    sf::Color Squircle::getFillColor() const
    {
        return m_fillColor;
    }

    void Squircle::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        if (s_isShaderLoaded)
        {
            sf::Vector2f currentScale = getScale();
            sf::Vector2f actualSize = {m_size.x * currentScale.x, m_size.y * currentScale.y};

            float padding = 0.f;
            if (m_shadowColor.a > 0)
            {
                padding = (m_shadowBlur + std::max(std::abs(m_shadowOffset.x), std::abs(m_shadowOffset.y))) * 2.0f;
            }
            sf::Vector2f totalSize = m_size + sf::Vector2f(padding, padding);
            sf::Vector2f actualTotalSize = {totalSize.x * currentScale.x, totalSize.y * currentScale.y};

            sf::Glsl::Vec4 actualRadii(
                m_radii.x * currentScale.x, m_radii.y * currentScale.x,
                m_radii.z * currentScale.x, m_radii.w * currentScale.x
            );

            float actualOutline = m_outlineThickness * currentScale.x;
            sf::Vector2f screenPos = states.transform.transformPoint(0.f, 0.f);
            float finalAlpha = (m_fillColor.a / 255.0f) * m_fillAlpha;

            s_shader.setUniform("u_size", actualSize);
            s_shader.setUniform("u_totalSize", actualTotalSize);
            s_shader.setUniform("u_radii", actualRadii);
            s_shader.setUniform("u_fillColor", sf::Glsl::Vec4(m_fillColor));
            s_shader.setUniform("u_fillAlpha", finalAlpha);

            s_shader.setUniform("u_outlineColor", sf::Glsl::Vec4(m_outlineColor));
            s_shader.setUniform("u_outlineThickness", actualOutline);
            s_shader.setUniform("u_power", m_power);

            s_shader.setUniform("u_shadowColor", sf::Glsl::Vec4(m_shadowColor));
            s_shader.setUniform("u_shadowBlur", m_shadowBlur * currentScale.x);
            s_shader.setUniform("u_shadowOffset", sf::Vector2f(m_shadowOffset.x * currentScale.x, m_shadowOffset.y * currentScale.y));

            if (m_bakedTexture != nullptr)
            {
                s_shader.setUniform("u_bakedBlurTexture", *m_bakedTexture);
                s_shader.setUniform("u_screenPos", screenPos);
                s_shader.setUniform("u_resolution", m_resolution);
            }
            else
            {
                s_shader.setUniform("u_resolution", sf::Vector2f(0.f, 0.f));
            }

            states.shader = &s_shader;

            sf::RectangleShape drawRect(totalSize);
            drawRect.setOrigin(totalSize / 2.0f);
            drawRect.setPosition(m_size / 2.0f);
            drawRect.setTextureRect(sf::IntRect(0, 0, 1, 1));

            target.draw(drawRect, states);
        }
        else
        {
            target.draw(m_rect, states);
        }
    }
}
