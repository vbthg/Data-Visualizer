#pragma once
#include <SFML/Graphics.hpp>

class State
{
public:
    virtual ~State() {}

    // Init tài nguyên khi vào state
    virtual void init() = 0;

    // Xử lý input (Chuột, Phím)
    virtual void handleInput(sf::Event& event) = 0;

    // Logic game (Animation, tính toán)
    virtual void update(float dt) = 0;

    // Vẽ
    virtual void draw() = 0;
};
