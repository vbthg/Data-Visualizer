#include "Heap.h"
#include "TimelineManager.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace DS
{
    Heap::Heap() : m_isMaxHeap(true), m_nextId(0)
    {
    }

    sf::Vector2f Heap::calculateNodePos(int index)
    {
        if(index == 0) return { 800.0f, 120.0f };

        int level = static_cast<int>(std::log2(index + 1));
        int posInLevel = index - (static_cast<int>(std::pow(2, level)) - 1);
        int totalNodesInLevel = static_cast<int>(std::pow(2, level));

        // Tầng càng sâu thì khoảng cách ngang giữa các node càng thu hẹp lại
        float offset = (BASE_X_GAP / std::pow(2, level - 1));
        float startX = 800.0f - (totalNodesInLevel - 1) * offset / 2.0f;

        float x = startX + posInLevel * offset;
        float y = 120.0f + level * LEVEL_HEIGHT;

        return { x, y };
    }

    void Heap::saveState(std::string title, std::string subtitle, GUI::Scenario scenario,
                         std::string macroKey, int line, int activeIdx, int targetIdx)
    {
        if(!m_timeline) return;

        auto snap = std::make_shared<Core::ISnapshot>();

        // 1. Snapshot Nodes
        for(int i = 0; i < (int)m_data.size(); ++i)
        {
            Core::NodeState ns;
            ns.id = m_data[i].id; // ID duy nhất đi theo giá trị để Orbital hoạt động
            ns.value = std::to_string(m_data[i].value);
            ns.position = calculateNodePos(i);
            ns.transition = Core::TransitionType::Orbital;
            ns.scale = 1.0f;
            ns.opacity = 1.0f;

            // Màu sắc mặc định: Off-white Apple
            ns.fillColor = sf::Color(242, 242, 247);
            ns.outlineColor = sf::Color(200, 200, 200, 180);
            ns.textColor = sf::Color(60, 60, 67);

            if(i == activeIdx)
            {
                ns.fillColor = sf::Color(0, 122, 255); // Blue
                ns.textColor = sf::Color::White;
            }
            else if(i == targetIdx)
            {
                ns.fillColor = sf::Color(255, 149, 0); // Orange
                ns.textColor = sf::Color::White;
            }

            snap->nodeStates.push_back(ns);
        }

        // 2. Snapshot Edges
        for(int i = 1; i < (int)m_data.size(); ++i)
        {
            Core::EdgeState es;
            es.startNodeId = m_data[getParent(i)].id;
            es.endNodeId = m_data[i].id;
            es.fillProgress = 1.0f;
            es.opacity = 0.5f;
            es.thickness = 3.0f;
            snap->edgeStates.push_back(es);
        }

        // 3. Notch & Code Context
        snap->notchData.title = title;
        snap->notchData.subtitle = subtitle;
        snap->notchData.scenario = scenario;

        snap->codeData.macroKey = macroKey;
        snap->codeData.pseudoCodeLine = line;

        // Variable States
        if(activeIdx != -1)
            snap->codeData.variableStates.push_back({"current_idx", std::to_string(activeIdx)});
        if(targetIdx != -1)
            snap->codeData.variableStates.push_back({"target_idx", std::to_string(targetIdx)});

        m_timeline->addSnapshot(snap);
    }

    void Heap::siftUp(int index, std::string macroTitle)
    {
        int curr = index;
        while(curr > 0)
        {
            int p = getParent(curr);
            bool violation = m_isMaxHeap ? (m_data[curr].value > m_data[p].value)
                                         : (m_data[curr].value < m_data[p].value);

            saveState(macroTitle, "Comparing child with parent", GUI::Scenario::Processing, "heap_insert", 2, curr, p);

            if(violation)
            {
                std::swap(m_data[curr], m_data[p]);
                saveState(macroTitle, "Swapping to maintain property", GUI::Scenario::Processing, "heap_insert", 4, p, curr);
                curr = p;
            }
            else
            {
                break;
            }
        }
    }

    void Heap::siftDown(int index, std::string macroTitle)
    {
        int curr = index;
        int n = (int)m_data.size();

        while(true)
        {
            int best = curr;
            int l = getLeft(curr);
            int r = getRight(curr);

            saveState(macroTitle, "Comparing node with its children", GUI::Scenario::Processing, "heap_extract", 4, curr);

            if(l < n && (m_isMaxHeap ? m_data[l].value > m_data[best].value : m_data[l].value < m_data[best].value))
                best = l;
            if(r < n && (m_isMaxHeap ? m_data[r].value > m_data[best].value : m_data[r].value < m_data[best].value))
                best = r;

            if(best != curr)
            {
                std::swap(m_data[curr], m_data[best]);
                saveState(macroTitle, "Swapping with the " + std::string(m_isMaxHeap ? "larger" : "smaller") + " child",
                          GUI::Scenario::Processing, "heap_extract", 8, best, curr);
                curr = best;
            }
            else
            {
                break;
            }
        }
    }

    void Heap::insert(int value)
    {
        m_timeline->onNewMacroStarted();
        std::string title = "Insert(" + std::to_string(value) + ")";

        // 1. Base snapshot: Preparing
        saveState(title, "Preparing to insert new value", GUI::Scenario::Processing, "heap_insert", 0);

        m_data.push_back({value, m_nextId++});
        int curr = (int)m_data.size() - 1;
        saveState(title, "Value added at the end of heap", GUI::Scenario::Processing, "heap_insert", 1, curr);

        // 2. Thinking Flow: Sift Up
        siftUp(curr, title);

        // 3. Base snapshot: Success
        saveState("Success", "Value " + std::to_string(value) + " has been inserted", GUI::Scenario::Success, "heap_insert", 7);
        m_timeline->onMacroFinished();
    }

    void Heap::extractRoot()
    {
        if(m_data.empty()) return;
        m_timeline->onNewMacroStarted();
        std::string title = "Extract Root";

        // 1. Base snapshot: Preparing
        saveState(title, "Preparing to extract root element", GUI::Scenario::Processing, "heap_extract", 0, 0);

        if(m_data.size() == 1)
        {
            m_data.pop_back();
        }
        else
        {
            m_data[0] = m_data.back();
            m_data.pop_back();
            saveState(title, "Last element moved to root", GUI::Scenario::Processing, "heap_extract", 2, 0);

            // 2. Thinking Flow: Sift Down
            siftDown(0, title);
        }

        // 3. Base snapshot: Success
        saveState("Success", "Root has been extracted", GUI::Scenario::Success, "heap_extract", 11);
        m_timeline->onMacroFinished();
    }

    void Heap::updateNodeValue(int index, int newValue)
    {
        if(index < 0 || index >= (int)m_data.size()) return;
        m_timeline->onNewMacroStarted();
        std::string title = "Update Index " + std::to_string(index);

        saveState(title, "Updating value to " + std::to_string(newValue), GUI::Scenario::Processing, "heap_insert", 0, index);

        int oldVal = m_data[index].value;
        m_data[index].value = newValue;

        if(m_isMaxHeap ? (newValue > oldVal) : (newValue < oldVal))
            siftUp(index, title);
        else
            siftDown(index, title);

        saveState("Success", "Value updated successfully", GUI::Scenario::Success, "heap_insert", 7);
        m_timeline->onMacroFinished();
    }

    void Heap::toggleHeapType()
    {
        m_timeline->onNewMacroStarted();
        m_isMaxHeap = !m_isMaxHeap;
        std::string title = m_isMaxHeap ? "Converting to Max-Heap" : "Converting to Min-Heap";

        saveState(title, "Re-building heap from bottom-up", GUI::Scenario::Processing, "heap_build", 0);

        for(int i = (int)m_data.size() / 2 - 1; i >= 0; --i)
        {
            siftDown(i, title);
        }

        saveState("Success", "Conversion completed", GUI::Scenario::Success, "heap_build", 12);
        m_timeline->onMacroFinished();
    }

    bool Heap::loadFromFile(const std::string& path)
    {
        std::ifstream file(path);
        if(!file.is_open()) return false;

        std::string type;
        file >> type;
        m_isMaxHeap = (type == "MAX");

        m_data.clear();
        m_nextId = 0;
        std::string line;
        std::getline(file, line); // Skip dư âm dòng đầu
        std::getline(file, line);
        std::stringstream ss(line);
        int val;
        while(ss >> val)
        {
            m_data.push_back({val, m_nextId++});
        }

        // Visualize quy trình Build Heap $O(N)$
        if(m_timeline)
        {
            m_timeline->onNewMacroStarted();
            saveState("Load & Build Heap", "Building heap from array bottom-up", GUI::Scenario::Processing, "heap_build", 0);

            for(int i = (int)m_data.size() / 2 - 1; i >= 0; --i)
            {
                siftDown(i, "Build Heap");
            }

            saveState("Success", "Heap loaded and built", GUI::Scenario::Success, "heap_build", 12);
            m_timeline->onMacroFinished();
        }

        return true;
    }

    std::vector<DS::Command> Heap::getCommands()
    {
        return {
            { L"\uE3D6", "Insert", DS::InputType::Integer, [this](DS::InputArgs args) { this->insert(args.iVal1); }},
            { L"\uE616", "Extract Root", DS::InputType::None, [this](DS::InputArgs args) { this->extractRoot(); }},
//            { L"\xef\x8b\xad", "Update", DS::InputType::TwoIntegers, [this](DS::InputArgs args) { this->updateNodeValue(args.iVal1, args.iVal2); }},
            { L"\uE094", "Toggle Type", DS::InputType::None, [this](DS::InputArgs args) { this->toggleHeapType(); }}
        };
    }
}
