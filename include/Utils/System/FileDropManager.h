// include/Utils/FileDropManager.h
#pragma once

#include <SFML/Window.hpp>
#include <string>
#include <vector>

class FileDropManager
{
public:
    static void init(sf::WindowHandle handle);
    static void shutdown();

    static bool isDragging();
    static sf::Vector2f getHoverPosition();
    static bool hasDroppedFiles();
    static std::vector<std::string> popDroppedFiles();

//    using DropCallback = std::function<void(const std::vector<std::string>&)>;
//    static DropCallback onDrop = nullptr;

//    static void setOnDropCallback(DropCallback callback)
//    {
//        onDrop = callback;
//    }
};
