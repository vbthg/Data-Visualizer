#pragma once
#include <SFML/Graphics.hpp>

namespace GUI
{
    class Squircle : public sf::Drawable, public sf::Transformable
    {
    public:
        Squircle(sf::Vector2f size = {0.f, 0.f});

        void setSize(sf::Vector2f size);
        void setRadius(float radius);
        void setRadius(float tl, float tr, float br, float bl);
        void setPower(float power);

        // Cập nhật hàm setFillColor để nhận thêm độ mờ (Alpha Tint)
        void setFillColor(sf::Color color, float alphaTint = 1.f);

        void setOutlineColor(sf::Color color);
        void setOutlineThickness(float thickness);

        void setShadow(sf::Color color, float blur, sf::Vector2f offset);

        // Hàm mới để truyền ảnh nền đã làm mờ
        void setBakedGlass(const sf::Texture* bakedTexture, sf::Vector2f windowResolution);
        void setContentTexture(const sf::Texture* texture);

        sf::Vector2f getSize() const;
        sf::FloatRect getLocalBounds() const;
        sf::FloatRect getGlobalBounds() const;
        sf::Color getFillColor() const; // Thêm dòng này

    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Thêm biến này (mặc định là 4.0 cho Squircle chuẩn)
        float m_power = 4.0f;

        sf::Vector2f m_size;
        sf::Glsl::Vec4 m_radii;
        sf::Color m_fillColor;
        float m_fillAlpha;

        // Thêm vào mục private:
        sf::Color m_shadowColor;
        float m_shadowBlur;
        sf::Vector2f m_shadowOffset;

        // [THÊM] 2 biến lưu trữ viền
        sf::Color m_outlineColor;
        float m_outlineThickness;

        const sf::Texture* m_contentTexture = nullptr;
        const sf::Texture* m_bakedTexture;
        sf::Vector2f m_resolution;

        sf::RectangleShape m_rect;

        static sf::Shader s_shader;
        static bool s_isShaderLoaded;
    };
}
