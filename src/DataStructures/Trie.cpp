#include "Trie.h"
#include <algorithm>

namespace DS
{
    Trie::Trie() : m_root(nullptr), m_nextId(0)
    {
        // Root là một node rỗng, không chứa ký tự
        m_root = new Node('\0', m_nextId++);
    }

    Trie::~Trie()
    {
        clearRecursive(m_root);
    }

    // --- COMMAND SYSTEM ---
    std::vector<Command> Trie::getCommands()
    {
        std::vector<Command> cmds;

        // Thêm từ (Sử dụng icon dấu cộng)
        cmds.push_back(Command(L"\uE3D6", InputType::String, [this](InputArgs args) {
            this->insert(args.sVal);
        }));

        // Tìm từ (Sử dụng icon kính lúp)
        cmds.push_back(Command(L"\uE30C", InputType::String, [this](InputArgs args) {
            this->search(args.sVal);
        }));

        // Xóa từ (Sử dụng icon thùng rác)
        cmds.push_back(Command(L"\uE32C", InputType::String, [this](InputArgs args) {
            this->remove(args.sVal);
        }));

        // Xóa sạch cây
        cmds.push_back(Command(L"\uEC54", InputType::None, [this](InputArgs args) {
            this->clear();
        }));

        return cmds;
    }

    // --- THAO TÁC CHÍNH ---

    void Trie::insert(const std::string& word)
    {
        if(word.empty()) return;
        m_timeline->onNewMacroStarted();
        // Khi bắt đầu insert

        Node* curr = m_root;
        // Dòng 0: Node* curr = root;
        createSnapshot(curr->id, "Start insert", "trie_insert", "Trie::INSERT", 0, {{"word", word}, {"curr", "Root"}});

        for(size_t i = 0; i < word.length(); ++i)
        {
            char c = word[i];
            std::string sChar(1, c);

            // Dòng 1: for each char c in word
            createSnapshot(curr->id, "Processing character: " + sChar, "trie_insert", "Trie::INSERT", 1, {{"word", word}, {"c", sChar}, {"i", std::to_string(i)}});

            if(curr->children.find(c) == curr->children.end())
            {
                // Dòng 2: if(curr->children[c] == null)
                createSnapshot(curr->id, "Branch '" + sChar + "' not found. Creating...", "trie_insert", "Trie::INSERT", 3, {{"c", sChar}});

                curr->children[c] = new Node(c, m_nextId++);

                // Dòng 3: curr->children[c] = new Node(c);
                createSnapshot(curr->children[c]->id, "New node created for '" + sChar + "'", "trie_insert", "Trie::INSERT", 5, {{"c", sChar}});
            }

            curr = curr->children[c];
            // Dòng 4: curr = curr->children[c];
            createSnapshot(curr->id, "Moving to next node", "trie_insert", "Trie::INSERT", 7, {{"curr", sChar}});
        }

        curr->isEndOfWord = true;
        // Dòng 5: curr->isEndOfWord = true;
        createSnapshot(curr->id, "Insert complete: '" + word + "'", "trie_insert", "Trie::INSERT", 9, {{"word", word}}, false);
    }

    bool Trie::search(const std::string& word)
    {
        if(word.empty()) return false;
        m_timeline->onNewMacroStarted();

        Node* curr = m_root;
        // Dòng 0: Node* curr = root;
        createSnapshot(curr->id, "Search start: " + word, "trie_search", "Trie::SEARCH", 0, {{"word", word}, {"curr", "Root"}});

        for(char c : word)
        {
            std::string sChar(1, c);

            // Dòng 1: for each char c in word
            createSnapshot(curr->id, "Processing character: " + sChar, "trie_search", "Trie::SEARCH", 1, {{"word", word}, {"c", sChar}});

            if(curr->children.find(c) == curr->children.end())
            {
                // Dòng 2: if(child == null) return false;
                createSnapshot(-1, "Character '" + sChar + "' not found!", "trie_search", "Trie::SEARCH", 5, {{"c", sChar}}, true);
                return false;
            }
            curr = curr->children[c];
            // Dòng 3: curr = curr->children[c];
            createSnapshot(curr->id, "Found '" + sChar + "', continuing...", "trie_search", "Trie::SEARCH", 7, {{"curr", sChar}});
        }

        // Dòng 4: return curr->isEndOfWord;
        bool found = curr->isEndOfWord;
        createSnapshot(curr->id, found ? "Match found!" : "Path exists but not a word", "trie_search", "Trie::SEARCH", 9, {{"found", found ? "true" : "false"}}, !found);

        return found;
    }

    void Trie::remove(const std::string& word)
    {
        if(word.empty()) return;
        m_timeline->onNewMacroStarted();

        bool deleted = false;
        removeRecursive(m_root, word, 0, deleted);

        if(deleted) createSnapshot(-1, "Removed '" + word + "' from Trie.", "trie_remove", "Trie::REMOVE", 4);
        else createSnapshot(-1, "Word '" + word + "' does not exist.", "trie_remove", "Trie::REMOVE", 4, {}, true);
    }

    void Trie::clear()
    {
        m_timeline->onNewMacroStarted();
        clearRecursive(m_root);
        m_root = new Node('\0', m_nextId++);
        createSnapshot(m_root->id, "Trie cleared.", "trie_clear", "Trie::CLEAR", 1);
    }

    // --- RECURSIVE HELPERS ---

    bool Trie::removeRecursive(Node* curr, const std::string& word, int index, bool& deleted)
    {
        if(!curr) return false;

        // Giai đoạn đi xuống (Traversal)
        createSnapshot(curr->id, "Checking character '" + std::string(1, word[index]) + "'", "trie_remove", "Trie::REMOVE", 11, {{"depth", std::to_string(index)}});

        if(index == word.length())
        {
            if(!curr->isEndOfWord) return false;

            deleted = true;
            curr->isEndOfWord = false;
            createSnapshot(curr->id, "Unmarked End of Word for '" + word + "'", "trie_remove", "Trie::REMOVE", 9);
            return curr->children.empty(); // Nếu không có con thì có thể xóa node này
        }

        char c = word[index];
        if(curr->children.find(c) == curr->children.end()) return false;

        bool canDeleteChild = removeRecursive(curr->children[c], word, index + 1, deleted);

        if(canDeleteChild)
        {
            // Giai đoạn đi lên (Unwinding/Backtracking)
            // Highlight node sắp bị xóa vĩnh viễn (Orange)
            createSnapshot(curr->children[c]->id, "Node '" + std::string(1, c) + "' is no longer part of any word. Deleting...", "trie_remove", "Trie::REMOVE", 14, {}, true);

            delete curr->children[c];
            curr->children.erase(c);

            // Tiếp tục cho phép xóa node cha nếu nó không phải kết thúc từ và không có con khác
            return !curr->isEndOfWord && curr->children.empty() && curr != m_root;
        }

        return false;
    }

    void Trie::clearRecursive(Node* node)
    {
        if(!node) return;
        for(auto& pair : node->children)
        {
            clearRecursive(pair.second);
        }
        delete node;
    }

    // --- SNAPSHOT SYSTEM ---

    void Trie::createSnapshot(int highlightNodeId, const std::string& message,
                              const std::string& macroKey, const std::string& opName,
                              int lineIdx, std::vector<std::pair<std::string, std::string>> vars,
                              bool isWarning)
    {
        auto snap = std::make_shared<Core::ISnapshot>();

        // 1. Tính toán Layout (Giữ nguyên logic cũ của bạn)
        computeLayout(m_root, 960.f, m_startY, 1400.f, snap->nodeStates, snap->edgeStates);

        // Gán dữ liệu cho PseudoCodeBox
        snap->macroKey = macroKey;          // VD: "trie_insert"
        snap->operationName = opName;       // VD: "TRIE: INSERT"
        snap->pseudoCodeLine = lineIdx;     // Dòng highlight
        snap->variableStates = vars;        // Danh sách biến số

        // 2. Highlight Node hiện tại
        for(auto& node : snap->nodeStates)
        {
            if(node.id == highlightNodeId)
            {
                if(isWarning)
                {
                    node.fillColor = sf::Color(255, 107, 107);
                    node.scale = 1.3f;
                }
                else
                {
                    node.fillColor = sf::Color(255, 212, 59);
                    node.scale = 1.15f;
                }
            }
        }

        // 3. NẠP DỮ LIỆU PSEUDOCODE VÀO ISNAPSHOT
        snap->logMessage = message;
        snap->pseudoCodeLine = lineIdx;  // Đây là biến mà PseudoCodeBox sẽ đọc

        // Giả sử bạn đã thêm biến này vào ISnapshot như tôi gợi ý ở trên
        // snap->variableStates = vars;

        if(m_timeline) m_timeline->addSnapshot(snap);
    }

    void Trie::computeLayout(Node* node, float x, float y, float horizontalRange,
                             std::vector<Core::NodeState>& nodes,
                             std::vector<Core::EdgeState>& edges)
    {
        if(!node) return;

        Core::NodeState state;
        state.id = node->id;
        state.position = {x, y};
        state.value = (node == m_root) ? "Root" : std::string(1, node->character);

        // Apple-style: Nếu là điểm kết thúc từ, dùng viền khác biệt
        if(node->isEndOfWord)
        {
            state.outlineColor = sf::Color(52, 199, 89); // Green Apple
            state.subText = "End";
        }

        nodes.push_back(state);

        if(node->children.empty()) return;

        // Chia không gian cho các con
        int numChildren = (int)node->children.size();
        float childRange = horizontalRange / numChildren;
        float startX = x - (horizontalRange / 2.0f) + (childRange / 2.0f);

        int i = 0;
        for(auto const& [c, child] : node->children)
        {
            float childX = startX + i * childRange;
            float childY = y + 120.f; // Khoảng cách dọc cố định 120px

            edges.push_back({node->id, child->id});
            computeLayout(child, childX, childY, childRange, nodes, edges);
            i++;
        }
    }
}
