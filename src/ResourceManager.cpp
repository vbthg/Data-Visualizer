#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    textures.clear();
    fonts.clear();
    cursors.clear();
}

sf::Texture& ResourceManager::getTexture(const std::string& filename)
{
    auto it = textures.find(filename);
    if(it != textures.end())
    {
        return it->second;
    }

    sf::Texture& tex = textures[filename];
    if(!tex.loadFromFile(filename))
    {
        std::cerr << "ResourceManager: Failed to load texture " << filename << std::endl;
        sf::Image img;
        img.create(32, 32, sf::Color::Red);
        tex.loadFromImage(img);
    }
    tex.setSmooth(true);
    tex.generateMipmap();

    return tex;
}

sf::Font& ResourceManager::getFont(const std::string& filename)
{
    auto it = fonts.find(filename);
    if(it != fonts.end())
    {
        return it->second;
    }

    sf::Font& font = fonts[filename];
    if(!font.loadFromFile(filename))
    {
        std::cerr << "ResourceManager: Failed to load font " << filename << std::endl;
    }
    return font;
}

const sf::Cursor& ResourceManager::getCursor(CursorType type)
{
    if(cursors.find(type) != cursors.end())
    {
        return *cursors[type];
    }

    auto cursor = std::make_unique<sf::Cursor>();
    bool success = false;

    switch(type)
    {
        case CursorType::Arrow: success = cursor->loadFromSystem(sf::Cursor::Arrow); break;
        case CursorType::Hand:  success = cursor->loadFromSystem(sf::Cursor::Hand); break;
        case CursorType::Wait:  success = cursor->loadFromSystem(sf::Cursor::Wait); break;
        case CursorType::Text:  success = cursor->loadFromSystem(sf::Cursor::Text); break;
        case CursorType::Cross: success = cursor->loadFromSystem(sf::Cursor::Cross); break;
    }

    if(!success)
    {
        // Fallback về Arrow nếu hệ thống không hỗ trợ
        cursor->loadFromSystem(sf::Cursor::Arrow);
    }

    cursors[type] = std::move(cursor);
    return *cursors[type];
}
