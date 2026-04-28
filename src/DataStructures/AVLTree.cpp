#include "AVLTree.h"
#include <fstream>
#include <algorithm>

namespace DS
{
    AVLTree::AVLTree() : m_root(nullptr), m_nodeCounter(0) {}

    AVLTree::~AVLTree()
    {
        // Trong môi trường visualizer, ta chỉ cần reset con trỏ
        m_root = nullptr;
    }

    int AVLTree::getHeight(AVLNode* n) { return n ? n->height : -1; }
    int AVLTree::getBalance(AVLNode* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }
    void AVLTree::updateHeight(AVLNode* n) { if(n) n->height = 1 + std::max(getHeight(n->left), getHeight(n->right)); }

    std::vector<Command> AVLTree::getCommands()
    {
        std::vector<Command> cmds;
        cmds.push_back(Command(L"\uE3D6", "Insert", InputType::Integer, [this](InputArgs args) { this->insert(args.iVal1); }));
        cmds.push_back(Command(L"\uE32C", "Remove", InputType::Integer, [this](InputArgs args) { this->remove(args.iVal1); }));
        cmds.push_back(Command(L"\uE30C", "Search", InputType::Integer, [this](InputArgs args) { this->search(args.iVal1); }));
        cmds.push_back(Command(L"\uEC54", "Clear", InputType::None, [this](InputArgs args) { this->clear(); }));
        return cmds;
    }

    void AVLTree::createSnapshot(const std::string& title, const std::string& sub,
                                 GUI::Scenario sce, const std::string& key, int line,
                                 std::vector<std::pair<std::string, std::string>> vars,
                                 int focusId, std::set<int> warningIds)
    {
        auto snap = std::make_shared<Core::ISnapshot>();
        snap->notchData = { sce, title, sub, "", 0, 0 };
        snap->codeData = { key, line, vars };

        generateStates(m_root, {800.0f, 100.0f}, HORIZONTAL_SPACING, snap->nodeStates, snap->edgeStates, focusId, warningIds);
        m_timeline->addSnapshot(snap);
    }

    void AVLTree::generateStates(AVLNode* node, sf::Vector2f pos, float offset,
                                 std::vector<Core::NodeState>& nStates,
                                 std::vector<Core::EdgeState>& eStates,
                                 int focusId, std::set<int> warningIds)
    {
        if(!node) return;

        Core::NodeState ns;
        ns.id = node->id;
        ns.position = pos;
        ns.value = std::to_string(node->data);
        ns.isDraggable = true; // Cho phép kéo node trong đồ thị/cây

        // Hiển thị BF thay vì H
        int bf = getBalance(node);
        ns.subText = "bf=" + std::to_string(bf);

        // Hiệu ứng Focus: Xanh Apple + 1.15x
        if(ns.id == focusId)
        {
            ns.fillColor = sf::Color(0, 122, 255);
            ns.textColor = sf::Color::White;
            ns.scale = 1.15f;
        }

        // Hiệu ứng Cảnh báo lệch: Viền đỏ + 1.2x
        if(warningIds.count(ns.id))
        {
            ns.outlineColor = sf::Color::Red;
            ns.textColor = sf::Color::Red;
            ns.scale = 1.2f;
        }

        nStates.push_back(ns);

        if(node->left)
        {
            eStates.push_back({node->id, node->left->id});
            generateStates(node->left, {pos.x - offset, pos.y + VERTICAL_GAP}, offset * 0.45f, nStates, eStates, focusId, warningIds);
        }
        if(node->right)
        {
            eStates.push_back({node->id, node->right->id});
            generateStates(node->right, {pos.x + offset, pos.y + VERTICAL_GAP}, offset * 0.45f, nStates, eStates, focusId, warningIds);
        }
    }

    // --- PHÉP XOAY ---
    AVLNode* AVLTree::rightRotate(AVLNode* y, const std::string& title)
    {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    AVLNode* AVLTree::leftRotate(AVLNode* x, const std::string& title)
    {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // --- INSERT ---
    void AVLTree::insert(int val)
    {
        if(!m_timeline) return;
        m_timeline->onNewMacroStarted();
        std::string title = "Insert " + std::to_string(val);

        createSnapshot(title, "Preparing to insert...", GUI::Scenario::Idle, "avl_insert", 0, {{"val", std::to_string(val)}});
        m_root = insertRecursive(m_root, val, title);

        createSnapshot(title, "Insertion completed!", GUI::Scenario::Success, "avl_insert", 7, {{"val", std::to_string(val)}});
        m_timeline->onMacroFinished();
    }

    AVLNode* AVLTree::insertRecursive(AVLNode* node, int val, const std::string& title)
    {
        if(!node) return new AVLNode(val, m_nodeCounter++);

        if(val < node->data)
        {
            createSnapshot(title, "Go left: " + std::to_string(val) + " < " + std::to_string(node->data), GUI::Scenario::Processing, "avl_insert", 0, {{"curr", std::to_string(node->data)}}, node->id);
            node->left = insertRecursive(node->left, val, title);
        }
        else if(val > node->data)
        {
            createSnapshot(title, "Go right: " + std::to_string(val) + " > " + std::to_string(node->data), GUI::Scenario::Processing, "avl_insert", 0, {{"curr", std::to_string(node->data)}}, node->id);
            node->right = insertRecursive(node->right, val, title);
        }
        else return node;

        updateHeight(node);
        int balance = getBalance(node);

        if(std::abs(balance) >= 2)
        {
            std::set<int> warnings = {node->id};
            if(balance > 1)
            {
                warnings.insert(node->left->id);
                if(val < node->left->data) warnings.insert(node->left->left->id);
                else warnings.insert(node->left->right->id);
            }
            else
            {
                warnings.insert(node->right->id);
                if(val > node->right->data) warnings.insert(node->right->right->id);
                else warnings.insert(node->right->left->id);
            }
            createSnapshot(title, "Imbalance detected at " + std::to_string(node->data), GUI::Scenario::Warning, "avl_insert", 2, {{"bf", std::to_string(balance)}}, -1, warnings);

            if(balance > 1 && val < node->left->data) return rightRotate(node, title);
            if(balance < -1 && val > node->right->data) return leftRotate(node, title);
            if(balance > 1 && val > node->left->data)
            {
                node->left = leftRotate(node->left, title);
                return rightRotate(node, title);
            }
            if(balance < -1 && val < node->right->data)
            {
                node->right = rightRotate(node->right, title);
                return leftRotate(node, title);
            }
        }
        return node;
    }

    // --- REMOVE ---
    void AVLTree::remove(int val)
    {
        if(!m_timeline) return;
        m_timeline->onNewMacroStarted();
        std::string title = "Remove " + std::to_string(val);

        createSnapshot(title, "Preparing to remove...", GUI::Scenario::Idle, "avl_remove", 0, {{"val", std::to_string(val)}});
        m_root = removeRecursive(m_root, val, title);

        createSnapshot(title, "Removal completed!", GUI::Scenario::Success, "avl_remove", 6, {{"val", std::to_string(val)}});
        m_timeline->onMacroFinished();
    }

    AVLNode* AVLTree::removeRecursive(AVLNode* node, int val, const std::string& title)
    {
        if(!node) return nullptr;

        if(val < node->data)
        {
            createSnapshot(title, "Go left to find " + std::to_string(val), GUI::Scenario::Processing, "avl_remove", 1, {{"curr", std::to_string(node->data)}}, node->id);
            node->left = removeRecursive(node->left, val, title);
        }
        else if(val > node->data)
        {
            createSnapshot(title, "Go right to find " + std::to_string(val), GUI::Scenario::Processing, "avl_remove", 1, {{"curr", std::to_string(node->data)}}, node->id);
            node->right = removeRecursive(node->right, val, title);
        }
        else
        {
            createSnapshot(title, "Found node " + std::to_string(val), GUI::Scenario::Processing, "avl_remove", 3, {{"val", std::to_string(val)}}, node->id);
            if(!node->left || !node->right)
            {
                AVLNode* temp = node->left ? node->left : node->right;
                delete node;
                return temp;
            }
            else
            {
                AVLNode* temp = node->right;
                while(temp->left) temp = temp->left;
                node->data = temp->data;
                node->right = removeRecursive(node->right, temp->data, title);
            }
        }

        updateHeight(node);
        int balance = getBalance(node);

        if(std::abs(balance) >= 2)
        {
            std::set<int> warnings = {node->id};
            createSnapshot(title, "Re-balancing after removal...", GUI::Scenario::Warning, "avl_remove", 5, {{"bf", std::to_string(balance)}}, -1, warnings);

            if(balance > 1 && getBalance(node->left) >= 0) return rightRotate(node, title);
            if(balance > 1 && getBalance(node->left) < 0)
            {
                node->left = leftRotate(node->left, title);
                return rightRotate(node, title);
            }
            if(balance < -1 && getBalance(node->right) <= 0) return leftRotate(node, title);
            if(balance < -1 && getBalance(node->right) > 0)
            {
                node->right = rightRotate(node->right, title);
                return leftRotate(node, title);
            }
        }
        return node;
    }

    void AVLTree::search(int val)
    {
        if(!m_timeline) return;
        m_timeline->onNewMacroStarted();
        std::string title = "Search " + std::to_string(val);
        AVLNode* curr = m_root;
        while(curr)
        {
            createSnapshot(title, "Searching " + std::to_string(curr->data), GUI::Scenario::Processing, "avl_search", 1, {{"val", std::to_string(val)}}, curr->id);
            if(val == curr->data)
            {
                createSnapshot(title, "Found!", GUI::Scenario::Success, "avl_search", 2, {{"val", std::to_string(val)}}, curr->id);
                m_timeline->onMacroFinished();
                return;
            }
            curr = (val < curr->data) ? curr->left : curr->right;
        }
        createSnapshot(title, "Not found", GUI::Scenario::Error, "avl_search", 3, {{"val", std::to_string(val)}});
        m_timeline->onMacroFinished();
    }

    void AVLTree::clear()
    {
        m_root = nullptr;
        m_nodeCounter = 0;
        createSnapshot("Clear", "Tree cleared.", GUI::Scenario::Idle, "", 0, {});
    }

    bool AVLTree::loadFromFile(const std::string& path)
    {
        if(!m_timeline) return false;
        m_timeline->onNewMacroStarted();
        createSnapshot("Importing data", "Reading: " + path, GUI::Scenario::Processing, "", 0, {{"file", path}});

        std::ifstream ifs(path);
        if(!ifs.is_open())
        {
            createSnapshot("Import failed", "Invalid file path.", GUI::Scenario::Error, "", 0, {});
            m_timeline->onMacroFinished();
            return false;
        }

        int val;
        while(ifs >> val) m_root = insertRecursive(m_root, val, "Importing");

        createSnapshot("Import success", "Data loaded.", GUI::Scenario::Success, "", 0, {});
        m_timeline->onMacroFinished();
        return true;
    }
}
