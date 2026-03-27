#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <string>
#include <utility>

#include "RoundedRectangleShape.h"
#include "Spring.h"

namespace GUI
{
    struct Token
    {
        sf::Text text;
    };

    struct VariableUI
    {
        sf::Text nameText;
        sf::Text valueText;
        Utils::Physics::Spring bounceSpring;
    };

    class PseudoCodeBox
    {
    public:
        PseudoCodeBox(const sf::Font& font, float windowWidth, float windowHeight);
        ~PseudoCodeBox();

        void loadCode(const std::string& title, const std::vector<std::string>& codeLines);
        void updateStep(int currentLineIndex, const std::vector<std::pair<std::string, std::string>>& variables);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void update(float dt);
        void draw(sf::RenderWindow& window);
        void onResize(float windowWidth, float windowHeight);

        void toggleState();

        bool isOpen() { return m_isExpanded; }

    private:
        const sf::Font& m_font;
        float m_windowWidth;
        float m_windowHeight;
        bool m_isExpanded;

        // --- Visual Components ---
        RoundedRectangleShape m_background;
        RoundedRectangleShape m_highlighter;
        sf::Text m_titleText;
        sf::Text m_collapsedIconText; // Chữ "</>" hiện lúc thu nhỏ

        // --- Data ---
        std::vector<std::vector<Token>> m_parsedCode;
        std::vector<VariableUI> m_variables;
        int m_currentLine;

        // --- Physics Springs (Morphing System) ---
        Utils::Physics::Spring m_wSpring;
        Utils::Physics::Spring m_hSpring;
        Utils::Physics::Spring m_xSpring;
        Utils::Physics::Spring m_ySpring;
        Utils::Physics::Spring m_alphaSpring; // Làm mờ chữ khi thu nhỏ

        Utils::Physics::Spring m_highlightYSpring;
        Utils::Physics::Spring m_scrollYSpring;

        void parseLine(const std::string& line, int lineIndex);
        void drawVariables(sf::RenderWindow& window);
    };
}
