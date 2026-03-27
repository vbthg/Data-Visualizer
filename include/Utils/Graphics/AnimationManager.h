#pragma once
#include <vector>
#include <functional>
#include <string>

namespace GUI
{
    class DynamicIsland;
}

namespace Utils
{
    struct AnimationStep
    {
        std::string logMessage = "";
        std::function<void()> onStart;
        std::function<void(float)> onUpdate;
        std::function<void()> onFinish;
        float duration = 0.5f;
    };

    class AnimationManager
    {
    private:
        std::vector<AnimationStep> m_steps;
        int m_currentIndex;
        float m_currentTime;
        bool m_isPlaying;
        float m_timeScale;
        GUI::DynamicIsland* m_island;


    public:
        AnimationManager();

        void setDynamicIsland(GUI::DynamicIsland* island);
        void setTimeScale(float scale);
        float getTimeScale() const;

        void addStep(const AnimationStep& step);
        void clear();

        void play();
        void pause();
        void stepForward();
        void finishAll();


        void update(float dt);
        bool isDone() const;
    };
}
