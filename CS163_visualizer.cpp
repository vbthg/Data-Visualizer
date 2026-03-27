#include "Application.h"
//#include "ResourceManager.h"
//
//
//void preload_assets()
//{
//    auto &res =
//}
//

int main()
{
    Application app;
    app.run();
    return 0;
}

//
//
//
//#include <SFML/Graphics.hpp>
//#include "FileDropManager.h"
//#include "NotchManager.h"
//
//int main()
//{
//    sf::RenderWindow window(sf::VideoMode(1920, 1080), "NotchManager Sandbox");
//    window.setFramerateLimit(60);
//
//    GUI::NotchManager::getInstance().init(sf::Vector2f(1920.f, 1080.f));
//
//    FileDropManager::init(window.getSystemHandle());
//
//    sf::Clock clock;
//
//    bool isSimulating = false;
//    int currentStep = 0;
//    int totalSteps = 5;
//    float stepTimer = 0.f;
//    float stepDuration = 1.2f;
//    bool wasDragging = false;
//
//    // --- KHAI BÁO THÊM BIẾN NÀY BÊN NGOÀI VÒNG LẶP WHILE ---
//float dragExitTimer = 0.f;
//const float DRAG_EXIT_DELAY = 0.15f; // Đợi 0.15 giây mới thực sự tắt Notch
//
//    while(window.isOpen())
//    {
//        float dt = clock.restart().asSeconds();
//        sf::Event event;
//
//        while(window.pollEvent(event))
//        {
//            if(event.type == sf::Event::Closed)
//            {
//                window.close();
//            }
//            if(event.type == sf::Event::KeyPressed)
//            {
//                if(event.key.code == sf::Keyboard::Num1)
//                {
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
//                }
//                if(event.key.code == sf::Keyboard::Num2)
//                {
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Processing, "Searching...", "Comparing 15 > 10", "S");
//                }
//                if(event.key.code == sf::Keyboard::Num3)
//                {
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Success, "Success!", "Value found", "C");
//                }
//                if(event.key.code == sf::Keyboard::Num4)
//                {
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Error, "Error", "Value already exists", "X");
//                }
//                if(event.key.code == sf::Keyboard::Num5)
//                {
//                    isSimulating = true;
//                    currentStep = 1;
//                    stepTimer = 0.f;
//
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Processing, "Inserting Node", "Step 1: Finding position", "I");
//                    GUI::NotchManager::getInstance().updateStep(currentStep, totalSteps, stepDuration);
//                }
//            }
//        }
//
//        // 2. main.cpp
//        bool isDragging = FileDropManager::isDragging();
//
//        if (isDragging && !wasDragging) {
//            GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::FileTray, "file tray", "", "\xef\x84\x9e");
//        }
//        else if (!isDragging && wasDragging) {
//            if (!FileDropManager::hasDroppedFiles()) {
//                GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
//            }
//        }
//
//        wasDragging = isDragging;
//
//        /*
//        bool isDragging = FileDropManager::isDragging();
//
//        if(isDragging)
//        {
//            dragExitTimer = DRAG_EXIT_DELAY; // Reset bộ đếm khi đang kéo
//            if (!wasDragging)
//            {
//                // Chỉ gọi 1 lần khi vừa chạm vào
//                GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::FileTray, "file tray", "Drop to import", "\xef\x84\x9e");
//            }
//        }
//
//        else if(!isDragging && wasDragging)
//        {
//            dragExitTimer -= dt; // Bắt đầu đếm ngược
//            if(dragExitTimer <= 0.f)
//            {
//                if(FileDropManager::hasDroppedFiles())
//                {
//                    std::vector<std::string> files = FileDropManager::popDroppedFiles();
//                    if(!files.empty())
//                    {
//                        std::string filePath = files[0];
//                        GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Processing, "Loading...", filePath, "\xef\x84\x9e");
//
//                        isSimulating = true;
//                        currentStep = 1;
//                        stepTimer = 0.f;
//                        totalSteps = 10;
//                        stepDuration = 0.15f;
//                        GUI::NotchManager::getInstance().updateStep(currentStep, totalSteps, stepDuration);
//                    }
//                }
//                else
//                {
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
//                }
//            }
//            wasDragging = false;
//
//        }
//        if (isDragging) wasDragging = true;
//
////        wasDragging = isDragging;
//*/
//
//        if(isSimulating)
//        {
//            stepTimer += dt;
//            if(stepTimer >= stepDuration)
//            {
//                stepTimer = 0.f;
//                currentStep++;
//
//                if(currentStep > totalSteps)
//                {
//                    isSimulating = false;
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Success, "Operation Complete", "File loaded successfully", "C");
//                }
//                else
//                {
//                    std::string subtitle = "Parsing line " + std::to_string(currentStep * 10);
//                    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Processing, "Loading...", subtitle, "\xef\x84\x9e");
//                    GUI::NotchManager::getInstance().updateStep(currentStep, totalSteps, stepDuration);
//                }
//            }
//        }
//
//        GUI::NotchManager::getInstance().update(dt);
//
//        window.clear(sf::Color(240, 240, 240));
//        window.draw(GUI::NotchManager::getInstance());
//        window.display();
//    }
//
//    FileDropManager::shutdown();
//    return 0;
//}
