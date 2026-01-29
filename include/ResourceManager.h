#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <memory>

enum class CursorType
{
    Arrow,
    Hand,
    Wait,
    Text,
    Cross
};

class ResourceManager
{
public:
    // Singleton Accessor
    static ResourceManager& getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    // Xóa Copy Constructor và Assignment Operator để đảm bảo duy nhất
    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&) = delete;

    sf::Texture& getTexture(const std::string& filename);
    sf::Font& getFont(const std::string& filename);
    const sf::Cursor& getCursor(CursorType type);

private:
    // Private Constructor
    ResourceManager();
    ~ResourceManager();

    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
    std::map<CursorType, std::unique_ptr<sf::Cursor>> cursors;
};
