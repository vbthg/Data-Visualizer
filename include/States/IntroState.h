#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include <string>
#include "State.h"
#include "Smoothing.h"
#include "ResourceManager.h"
#include "Button.h"

// Struct quản lý hiệu ứng chữ
struct AppleText
{
    sf::Text text;
    float alpha;
    float targetAlpha;
    float offsetY;
    float targetOffsetY;
    bool isVisible;

    void setup(const sf::Font& font, const std::string& content, unsigned int size, sf::Vector2f centerPos);
    void show();
    void update(float dt);
    void draw(sf::RenderWindow& window);
};

// Struct quản lý Node
struct IntroNode
{
    int id;
    sf::Sprite sprite;
    sf::Vector2f currentPos;
    sf::Vector2f targetPos;
    IntroNode* parent;

    IntroNode(int id, const sf::Texture& tex, sf::Vector2f startPos);
    void update(float dt);
};

// Keyframe cho hoạt cảnh
struct AnimationStep
{
    std::vector<sf::Vector2f> nodePositions;
    std::vector<int> parentIndices;
    sf::Vector2f camFocus;
    float camZoom;
    float duration;
};

class IntroState : public State
{
public:
    IntroState(sf::RenderWindow& window, std::stack<State*>& states);
    virtual ~IntroState();

    void init();
    void handleInput(sf::Event& event);
    void update(float dt);
    void draw();

private:
    sf::RenderWindow& window;
    std::stack<State*>& states;

    // Objects
    std::vector<IntroNode> nodes;

    // Texts
    AppleText textBalance;
    AppleText textDynamic;
    AppleText textRespond;

    // Button
    GUI::Button* btnStart;
    float btnAlpha;

    // Animation Flow
    std::vector<AnimationStep> steps;
    int currentStepIndex;
    float stepTimer;
    bool isFinished;

    // Camera
    sf::View view;
    sf::Vector2f currentCamCenter;
    sf::Vector2f targetCamCenter;
    float currentZoom;
    float targetZoom;

    // --- TRANSITION VARIABLES (THÊM MỚI) ---
    bool isTransitioning;           // Đang chuyển cảnh?
    float transitionTimer;          // Đếm thời gian chuyển
    float transitionAlpha;          // Độ trong suốt màn trắng
    sf::RectangleShape overlayRect; // Lớp phủ trắng

    // Helpers
    void nextStep();
    void setupKeyframes();
    void skipIntro();
};
