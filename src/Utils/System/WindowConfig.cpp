#include "WindowConfig.h"

namespace Utils
{
    namespace System
    {
        sf::FloatRect getLetterboxViewport(int width, int height)
        {
            float targetRatio = DESIGN_WIDTH / DESIGN_HEIGHT;
            float windowRatio = (float)width / (float)height;

            float sizeX = 1.0f;
            float sizeY = 1.0f;
            float posX = 0.0f;
            float posY = 0.0f;

            if(windowRatio >= targetRatio)
            {
                sizeX = targetRatio / windowRatio;
                posX = (1.0f - sizeX) / 2.0f;
            }
            else
            {
                sizeY = windowRatio / targetRatio;
                posY = (1.0f - sizeY) / 2.0f;
            }

            return sf::FloatRect(posX, posY, sizeX, sizeY);
        }

        void applyLetterboxView(sf::RenderWindow& window, int width, int height)
        {
            sf::View view(sf::FloatRect(0.f, 0.f, DESIGN_WIDTH, DESIGN_HEIGHT));
            view.setViewport(getLetterboxViewport(width, height));
            window.setView(view);
        }

        void updateCustomView(sf::View& view, int width, int height)
        {
            view.setViewport(getLetterboxViewport(width, height));
        }
    }
}
