#include "Trie.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace DS
{
    Trie::Trie() : m_root(nullptr), m_nextId(0)
    {
        m_root = new Node('\0', m_nextId++);
    }

    Trie::~Trie()
    {
        clearRecursive(m_root);
    }

    float Trie::calculateSubtreeWidth(Node* node, std::map<int, float>& widthMap)
    {
        if(!node) return 0.f;
        if(node->children.empty())
        {
            widthMap[node->id] = NODE_GAP;
            return NODE_GAP;
        }

        float totalWidth = 0.f;
        for(auto const& [c, child] : node->children)
        {
            totalWidth += calculateSubtreeWidth(child, widthMap);
        }

        widthMap[node->id] = totalWidth;
        return totalWidth;
    }

    void Trie::createSnapshot(GUI::Scenario scenario, const std::string& title, const std::string& subtitle,
                               const std::string& macroKey, int lineIdx, int highlightNodeId,
                               std::vector<std::pair<std::string, std::string>> vars)
    {
        auto snap = std::make_shared<Core::ISnapshot>();

        // 1. Tính toán Layout và gán trạng thái Node/Edge
        computeLayout(m_root, 800.f, m_startY, 1400.f, snap->nodeStates, snap->edgeStates);

        // 2. Notch Context
        snap->notchData.scenario = scenario;
        snap->notchData.title = title;
        snap->notchData.subtitle = subtitle;

        if (scenario == GUI::Scenario::Success) snap->notchData.iconCode = "\xef\x80\x8c";
        else if (scenario == GUI::Scenario::Error) snap->notchData.iconCode = "\xef\x81\xb1";
        else snap->notchData.iconCode = "\xef\x84\x9e";

        // 3. Code Context
        snap->codeData.macroKey = macroKey;
        snap->codeData.pseudoCodeLine = lineIdx;
        snap->codeData.variableStates = vars;

        // 4. Highlight logic cho Node đang xét
        for (auto& node : snap->nodeStates)
        {
            node.isDraggable = false; // Bật tính năng kéo thả
            if (node.id == highlightNodeId)
            {
                node.fillColor = (scenario == GUI::Scenario::Error) ? COLOR_ERROR : COLOR_FOCUS;
                node.scale = 1.2f;
                node.textColor = (scenario == GUI::Scenario::Error) ? sf::Color::White : sf::Color::Black;
            }
        }

        if (m_timeline) m_timeline->addSnapshot(snap);
    }

    void Trie::insert(const std::string& word)
    {
        if (word.empty()) return;
        m_timeline->onNewMacroStarted();

        Node* curr = m_root;
        std::string title = "Insert Word: " + word;

        // Base snapshot
        createSnapshot(GUI::Scenario::Processing, title, "Initializing search from Root", "trie_insert", 0, curr->id, {{"word", word}});

        for (size_t i = 0; i < word.length(); ++i)
        {
            char c = word[i];
            std::string sChar(1, c);
            std::vector<std::pair<std::string, std::string>> vars = {{"word", word}, {"c", sChar}, {"index", std::to_string(i)}};

            createSnapshot(GUI::Scenario::Processing, title, "Checking character '" + sChar + "'", "trie_insert", 1, curr->id, vars);

            if (curr->children.find(c) == curr->children.end())
            {
                createSnapshot(GUI::Scenario::Processing, title, "Character '" + sChar + "' not found, creating node", "trie_insert", 3, curr->id, vars);
                curr->children[c] = new Node(c, m_nextId++);

                createSnapshot(GUI::Scenario::Processing, title, "Node created for '" + sChar + "'", "trie_insert", 6, curr->children[c]->id, vars);
            }
            else
            {
                createSnapshot(GUI::Scenario::Processing, title, "Character '" + sChar + "' found, moving down", "trie_insert", 8, curr->children[c]->id, vars);
            }
            curr = curr->children[c];
        }

        curr->isEndOfWord = true;
        createSnapshot(GUI::Scenario::Success, title, "Word '" + word + "' inserted successfully", "trie_insert", 10, curr->id, {{"isEndOfWord", "true"}});

        m_timeline->onMacroFinished();
    }

    bool Trie::search(const std::string& word)
    {
        m_timeline->onNewMacroStarted();
        Node* curr = m_root;
        std::string title = "Search Word: " + word;

        createSnapshot(GUI::Scenario::Processing, title, "Starting search from Root", "trie_search", 0, curr->id);

        for (size_t i = 0; i < word.length(); ++i)
        {
            char c = word[i];
            std::string sChar(1, c);
            createSnapshot(GUI::Scenario::Processing, title, "Looking for '" + sChar + "'", "trie_search", 1, curr->id, {{"c", sChar}});

            if (curr->children.find(c) == curr->children.end())
            {
                createSnapshot(GUI::Scenario::Error, title, "Character '" + sChar + "' not found. Search failed", "trie_search", 5, curr->id);
                m_timeline->onMacroFinished();
                return false;
            }
            curr = curr->children[c];
        }

        bool found = curr->isEndOfWord;
        createSnapshot(found ? GUI::Scenario::Success : GUI::Scenario::Warning, title,
                       found ? "Word found!" : "Prefix found, but not a complete word",
                       "trie_search", 10, curr->id, {{"found", found ? "true" : "false"}});

        m_timeline->onMacroFinished();
        return found;
    }

    void Trie::remove(const std::string& word)
    {
        m_timeline->onNewMacroStarted();
        bool deleted = false;
        std::string title = "Remove Word: " + word;

        createSnapshot(GUI::Scenario::Processing, title, "Locating word to remove", "trie_remove", 0, m_root->id);
        removeRecursive(m_root, word, 0, deleted);

        if (deleted)
            createSnapshot(GUI::Scenario::Success, title, "Word '" + word + "' removed", "trie_remove", 10, m_root->id);
        else
            createSnapshot(GUI::Scenario::Error, title, "Word '" + word + "' not found", "trie_remove", 4, m_root->id);

        m_timeline->onMacroFinished();
    }

    bool Trie::removeRecursive(Node* curr, const std::string& word, int index, bool& deleted)
    {
        if (!curr) return false;

        std::string title = "Remove Word: " + word;
        if (index == (int)word.length())
        {
            if (curr->isEndOfWord)
            {
                curr->isEndOfWord = false;
                deleted = true;
                createSnapshot(GUI::Scenario::Processing, title, "Unmarked EndOfWord", "trie_remove", 9, curr->id);
                return curr->children.empty();
            }
            return false;
        }

        char c = word[index];
        if (curr->children.find(c) == curr->children.end()) return false;

        bool shouldDeleteChild = removeRecursive(curr->children[c], word, index + 1, deleted);

        if (shouldDeleteChild)
        {
            createSnapshot(GUI::Scenario::Processing, title, "Deleting redundant node '" + std::string(1, c) + "'", "trie_remove", 15, curr->children[c]->id);
            delete curr->children[c];
            curr->children.erase(c);
            return curr->children.empty() && !curr->isEndOfWord;
        }

        return false;
    }

    bool Trie::loadFromFile(const std::string& path)
    {
        m_timeline->onNewMacroStarted();
        createSnapshot(GUI::Scenario::Processing, "Importing Data", "Reading: " + path, "trie_insert", -1);

        std::ifstream file(path);
        if (!file.is_open())
        {
            createSnapshot(GUI::Scenario::Error, "Import Failed", "Could not open file", "trie_insert", -1);
            m_timeline->onMacroFinished();
            return false;
        }

        std::string word;
        int count = 0;
        while (file >> word)
        {
            // Để tránh quá tải snapshot khi load file lớn, ta chỉ insert logic
            // Nếu muốn visualize từng từ khi load, hãy gọi this->insert(word)
            Node* curr = m_root;
            for (char c : word)
            {
                if (curr->children.find(c) == curr->children.end())
                    curr->children[c] = new Node(c, m_nextId++);
                curr = curr->children[c];
            }
            curr->isEndOfWord = true;
            count++;
        }

        createSnapshot(GUI::Scenario::Success, "Import Success", "Loaded " + std::to_string(count) + " words", "trie_insert", -1);
        m_timeline->onMacroFinished();
        return true;
    }

    void Trie::computeLayout(Node* node, float x, float y, float horizontalRange,
                         std::vector<Core::NodeState>& nodes,
                         std::vector<Core::EdgeState>& edges)
    {
        if(!node) return;

        std::map<int, float> widthMap;
        calculateSubtreeWidth(m_root, widthMap);

        // Sử dụng một Lambda hoặc hàm helper để đệ quy đặt vị trí
        std::function<void(Node*, float, float)> placeNodes = [&](Node* curr, float curX, float curY)
        {
            Core::NodeState state;
            state.id = curr->id;
            state.position = {curX, curY};
            state.value = (curr == m_root) ? "Root" : std::string(1, curr->character);
            state.isDraggable = true;
            state.fillColor = COLOR_DEFAULT;
            state.outlineColor = curr->isEndOfWord ? COLOR_SUCCESS : sf::Color(200, 200, 200);
            state.subText = curr->isEndOfWord ? "End" : "";
            nodes.push_back(state);

            if(curr->children.empty()) return;

            float totalW = widthMap[curr->id];
            float leftX = curX - totalW / 2.0f;
            float accumulatedW = 0.f;

            for(auto const& [c, child] : curr->children)
            {
                float w = widthMap[child->id];
                float childX = leftX + accumulatedW + w / 2.0f;
                float childY = curY + 120.f;

                Core::EdgeState edge;
                edge.startNodeId = curr->id;
                edge.endNodeId = child->id;
                edge.thickness = 3.0f;
                edge.isFocused = false;
                edges.push_back(edge);

                placeNodes(child, childX, childY);
                accumulatedW += w;
            }
        };

        placeNodes(m_root, 960.f, m_startY); // 960.f là trung tâm màn hình Full HD
    }

    void Trie::clear()
    {
        m_timeline->onNewMacroStarted();
        clearRecursive(m_root);
        m_nextId = 0;
        m_root = new Node('\0', m_nextId++);

        createSnapshot(GUI::Scenario::Success, "Clear", "Trie has been reset", "trie_insert", -1, m_root->id);
        m_timeline->onMacroFinished();
    }

    void Trie::clearRecursive(Node* node)
    {
        if (!node) return;
        for (auto& pair : node->children) clearRecursive(pair.second);
        delete node;
    }

    std::vector<Command> Trie::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command(L"\uE3D6", "Insert", InputType::String, [this](InputArgs args) { this->insert(args.sVal); }));
        cmds.push_back(Command(L"\uE30C", "Search", InputType::String, [this](InputArgs args) { this->search(args.sVal); }));
        cmds.push_back(Command(L"\uE32C", "Remove", InputType::String, [this](InputArgs args) { this->remove(args.sVal); }));
        cmds.push_back(Command(L"\uEC54", "Clear", InputType::None, [this](InputArgs args) { this->clear(); }));
        // Thêm lệnh Load File
//        cmds.push_back(Command(L"\uE24D", "Load File", InputType::String, [this](InputArgs args) { this->loadFromFile(args.sVal); }));
        return cmds;
    }
}
