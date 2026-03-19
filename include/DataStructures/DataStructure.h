// DataStructure.h
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Command.h"

namespace GUI { class DynamicIsland; }

class DataStructure
{
public:
    virtual ~DataStructure() = default;

    virtual std::vector<DS::Command> getCommands() = 0;

    virtual void update(float dt) = 0;

    // ĐỔI THÀNH RenderTarget để có thể vẽ vào RenderTexture
    virtual void draw(sf::RenderTarget& target) = 0;

    virtual std::string getName() const = 0;

    virtual void bindDynamicIsland(GUI::DynamicIsland* island) {}

    virtual void setMousePosition(sf::Vector2f pos) {}

    // Thêm hàm này để các thuật toán tự căn chỉnh node vào tâm khung hình
    virtual void setViewport(sf::FloatRect rect) {}
};
