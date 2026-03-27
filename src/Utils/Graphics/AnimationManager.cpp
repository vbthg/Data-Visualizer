#include "AnimationManager.h"
#include "DynamicIsland.h"

namespace Utils
{
    AnimationManager::AnimationManager()
    {
        m_currentIndex = 0;
        m_currentTime = 0.0f;
        m_isPlaying = false;
        m_timeScale = 2.f;
        m_island = nullptr;
    }

    void AnimationManager::setDynamicIsland(GUI::DynamicIsland* island)
    {
        m_island = island;
    }

    void AnimationManager::setTimeScale(float scale)
    {
        if(scale > 0.01f)
        {
            m_timeScale = scale;
        }
    }

    float AnimationManager::getTimeScale() const
    {
        return m_timeScale;
    }

    void AnimationManager::addStep(const AnimationStep& step)
    {
        m_steps.push_back(step);
    }

    void AnimationManager::clear()
    {
        m_steps.clear();
        m_currentIndex = 0;
        m_currentTime = 0.0f;
        m_isPlaying = false;

        if(m_island)
        {
            m_island->hideMessage();
        }
    }

    void AnimationManager::play()
    {
        if(m_currentIndex < m_steps.size())
        {
            m_isPlaying = true;
        }
    }

    void AnimationManager::pause()
    {
        m_isPlaying = false;
    }

    // Thêm vào AnimationManager.h trước
    // void finishAll();

    void AnimationManager::finishAll()
    {
        if(m_steps.empty()) return;

        // Chạy xuyên qua tất cả các bước còn lại
        while(m_currentIndex < m_steps.size())
        {
            AnimationStep& step = m_steps[m_currentIndex];

            // Nếu bước này chưa kịp Start, hãy Start nó
            if(m_currentTime == 0.0f && step.onStart)
            {
                step.onStart();
            }

            // Ép Update về 1.0 (trạng thái cuối)
            if(step.onUpdate)
            {
                step.onUpdate(1.0f);
            }

            // Gọi Finish để hoàn tất logic thuật toán của bước đó
            if(step.onFinish)
            {
                step.onFinish();
            }

            m_currentIndex++;
            m_currentTime = 0.0f;
        }

        m_isPlaying = false;
        m_steps.clear();
        m_currentIndex = 0;

        if(m_island)
        {
            m_island->hideMessage();
        }
    }

    void AnimationManager::stepForward()
    {
        if(m_currentIndex < m_steps.size())
        {
            if(m_steps[m_currentIndex].onFinish)
            {
                m_steps[m_currentIndex].onFinish();
            }

            if(m_island && !m_steps[m_currentIndex].logMessage.empty())
            {
                m_island->hideMessage();
            }

            m_currentIndex++;
            m_currentTime = 0.0f;

            if(m_currentIndex < m_steps.size() && !m_isPlaying)
            {
                if(m_steps[m_currentIndex].onStart)
                {
                    m_steps[m_currentIndex].onStart();
                }

                if(m_island && !m_steps[m_currentIndex].logMessage.empty())
                {
                    m_island->showMessage(m_steps[m_currentIndex].logMessage);
                }
            }
        }
    }

    void AnimationManager::update(float dt)
    {
        if(!m_isPlaying || m_steps.empty() || m_currentIndex >= m_steps.size())
        {
            return;
        }

        AnimationStep& currentStep = m_steps[m_currentIndex];

        if(m_currentTime == 0.0f)
        {
            if(currentStep.onStart)
            {
                currentStep.onStart();
            }

            if(m_island && !currentStep.logMessage.empty())
            {
                m_island->showMessage(currentStep.logMessage);
            }
        }


        // Nếu còn quá nhiều bước (VD > 3), tự động tăng tốc độ để đuổi kịp logic
        float dynamicScale = m_timeScale;
        size_t remaining = m_steps.size() - m_currentIndex;
        if(remaining > 3) dynamicScale *= (remaining * 0.5f);

        m_currentTime += (dt * dynamicScale);
        float t = m_currentTime / currentStep.duration;

        if(t >= 1.0f)
        {
            t = 1.0f;

            if(currentStep.onUpdate) currentStep.onUpdate(t);
            if(currentStep.onFinish) currentStep.onFinish();

            if(m_island && !currentStep.logMessage.empty())
            {
                m_island->hideMessage();
            }

            m_currentTime = 0.0f;
            m_currentIndex++;

            if(m_currentIndex >= m_steps.size())
            {
                m_isPlaying = false;
            }
        }
        else
        {
            if(currentStep.onUpdate)
            {
                currentStep.onUpdate(t);
            }
        }
    }

    bool AnimationManager::isDone() const
    {
        return m_currentIndex >= m_steps.size();
    }
}
