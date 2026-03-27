#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    namespace System
    {
        const float DESIGN_WIDTH = 1920.0f;
        const float DESIGN_HEIGHT = 1080.0f;
        static const char* APP_TITLE = "Structure Visualizer";

        void applyLetterboxView(sf::RenderWindow& window, int width, int height);
        sf::FloatRect getLetterboxViewport(int width, int height);
        void updateCustomView(sf::View& view, int width, int height);
    }
}
