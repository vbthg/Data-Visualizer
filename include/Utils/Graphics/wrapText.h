#pragma once
#include <SFML/Graphics.hpp>

namespace Utils
{
    namespace Graphics
    {
        inline void wrapText(sf::Text& text, float maxWidth)
        {
            std::string string = text.getString();
            std::string wrapped;
            std::string line;
            std::string word;
            std::stringstream ss(string);

            while(ss >> word)
            {
                std::string testLine = line + (line.empty() ? "" : " ") + word;
                text.setString(testLine);

                if(text.getLocalBounds().width > maxWidth)
                {
                    wrapped += (wrapped.empty() ? "" : "\n") + line;
                    line = word;
                }
                else
                {
                    line = testLine;
                }
            }
            wrapped += (wrapped.empty() ? "" : "\n") + line;
            text.setString(wrapped);
        }
    }
}
