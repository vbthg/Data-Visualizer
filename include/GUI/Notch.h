#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "Spring.h"
#include "Waveform.h"
#include "ProgressBar.h"
#include "GUI/NotchContent.h"
#include "NotchEnums.h"

namespace GUI
{
    class Notch : public sf::Drawable, public sf::Transformable
    {
    public:
        Notch(sf::Vector2f size);

        void setSize(sf::Vector2f size);
        void setRadii(float bottomRadius, float topFlareRadius);
        void setFillColor(sf::Color color, float alphaTint = 1.0f);
        void setShadow(sf::Color color, float blur, sf::Vector2f offset);
        void setWaveformState(GUI::Waveform::State state);
        void setScaleTarget(float target);

        void changeContent(std::unique_ptr<NotchContent> nextContent);
        void update(float dt);

        sf::Vector2f getSize() const;
        sf::FloatRect getLocalBounds() const;
        sf::FloatRect getGlobalBounds() const;
        GUI::NotchContent* getContent() const { return m_currentContent.get(); }

        void setStepInfo(int current, int total, float duration);
        void setScenario(Scenario scenario)
        {
            m_currentScenario = scenario;
        }

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        Scenario m_currentScenario = Scenario::Idle; // Thêm biến này

        sf::Vector2f m_size;
        float m_radiusBottom;
        float m_radiusTopFlare;
        sf::Color m_fillColor;
        float m_fillAlpha;
        sf::Color m_shadowColor;
        float m_shadowBlur;
        sf::Vector2f m_shadowOffset;
        mutable sf::RectangleShape m_rect;

        GUI::Waveform m_waveform;
        GUI::ProgressBar m_progressBar;

        Utils::Physics::Spring2D m_sizeSpring;
        Utils::Physics::Spring m_radiusBottomSpring;
        Utils::Physics::Spring m_radiusTopFlareSpring;
        Utils::Physics::Spring m_scaleSpring; // Lò xo quản lý tỉ lệ phóng đại (1.0 là bình thường)

        std::unique_ptr<NotchContent> m_currentContent;
        std::unique_ptr<NotchContent> m_oldContent;
        bool m_isTransitioning = false;
        float m_transitionProgress = 0.0f;

        static sf::Shader s_shader;
        static bool s_isShaderLoaded;

        static std::string getShaderCode();
    };
}
