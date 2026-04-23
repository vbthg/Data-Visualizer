#include "AVLTree.h"
#include <algorithm>
#include <string>

namespace DS
{
    AVLTree::AVLTree() : m_root(nullptr), m_nextId(0)
    {
    }

    AVLTree::~AVLTree()
    {
        clearRecursive(m_root);
    }

    // --- COMMAND SYSTEM ---
    std::vector<Command> AVLTree::getCommands()
    {
        std::vector<Command> cmds;

        cmds.push_back(Command(L"\uE3D6", InputType::Integer, [this](InputArgs args) {  // insert
            this->insert(args.iVal1);
        }));

        cmds.push_back(Command("L\uE32C", InputType::Integer, [this](InputArgs args) {
            this->remove(args.iVal1);
        }));

        cmds.push_back(Command(L"\uE30C", InputType::Integer, [this](InputArgs args) {
            this->search(args.iVal1);
        }));

        cmds.push_back(Command(L"\uE3AA", InputType::TwoIntegers, [this](InputArgs args) {
            this->updateNodeValue(args.iVal1, args.iVal2);
        }));

        cmds.push_back(Command(L"\uEC54", InputType::None, [this](InputArgs args) {
            this->clear();
        }));

        return cmds;
    }

    // --- THAO TÁC CHÍNH ---

    void AVLTree::insert(int value)
    {
        m_timeline->onNewMacroStarted();
        bool isInserted = false;
        m_root = insertRecursive(m_root, value, isInserted);

        if(isInserted)
        {
            createSnapshot(-1, "Insert " + std::to_string(value) + " complete.");
        }
        else
        {
            createSnapshot(-1, "Value " + std::to_string(value) + " already exists.");
        }
    }

    void AVLTree::remove(int value)
    {
        m_timeline->onNewMacroStarted();
        bool isRemoved = false;
        m_root = removeRecursive(m_root, value, isRemoved);

        if(isRemoved)
        {
            createSnapshot(-1, "Remove " + std::to_string(value) + " complete.");
        }
        else
        {
            createSnapshot(-1, "Value " + std::to_string(value) + " not found.");
        }
    }

    bool AVLTree::search(int value)
    {
        m_timeline->onNewMacroStarted();
        Node* curr = m_root;

        while(curr)
        {
            createSnapshot(curr->id, "Searching: Comparing " + std::to_string(value) + " with " + std::to_string(curr->data));

            if(value == curr->data)
            {
                createSnapshot(curr->id, "Found " + std::to_string(value) + "!");
                return true;
            }

            curr = (value < curr->data) ? curr->left : curr->right;
        }

        createSnapshot(-1, "Value " + std::to_string(value) + " not found in tree.");
        return false;
    }

    void AVLTree::updateNodeValue(int oldVal, int newVal)
    {
        m_timeline->onNewMacroStarted();

        // 1. Tìm node cũ để highlight
        if(!search(oldVal)) return;

        // 2. Xóa node cũ
        bool isRemoved = false;
        m_root = removeRecursive(m_root, oldVal, isRemoved);
        createSnapshot(-1, "Updating: Removing old value " + std::to_string(oldVal));

        // 3. Chèn node mới
        bool isInserted = false;
        m_root = insertRecursive(m_root, newVal, isInserted);
        createSnapshot(-1, "Updating: Inserting new value " + std::to_string(newVal));
    }

    void AVLTree::clear()
    {
        m_timeline->onNewMacroStarted();
        clearRecursive(m_root);
        m_root = nullptr;
        createSnapshot(-1, "Tree cleared.");
    }

    // --- RECURSIVE HELPERS ---

AVLTree::Node*& AVLTree::insertRecursive(Node*& node, int value, bool& isInserted)
{
    if(!node)
    {
        isInserted = true;
        node = new Node(value, m_nextId++);
        createSnapshot(node->id, "New leaf created: " + std::to_string(value));
        return node;
    }

    createSnapshot(node->id, "Traversing: " + std::to_string(value));

    int childId = -1;
    if(value < node->data)
    {
        insertRecursive(node->left, value, isInserted);
        if(node->left) childId = node->left->id;
    }
    else if(value > node->data)
    {
        insertRecursive(node->right, value, isInserted);
        if(node->right) childId = node->right->id;
    }
    else return node;

    // --- GIAI ĐOẠN UNWINDING (QUAY LUI) ---
    updateHeight(node);

    // Nhịp: Pulse từ con truyền lên cha + Cha hiện màu vàng (Checking)
    createSnapshot(node->id, "Unwinding: Updating height and checking BF for " + std::to_string(node->data), false);

    balance(node);

    return node;
}

AVLTree::Node*& AVLTree::removeRecursive(Node*& node, int value, bool& isRemoved)
{
    if(!node)
    {
        return node;
    }

    // Nhịp 1: Duyệt tìm node cần xóa
    createSnapshot(node->id, "Searching for node " + std::to_string(value) + " to remove.");

    if(value < node->data)
    {
        removeRecursive(node->left, value, isRemoved);
    }
    else if(value > node->data)
    {
        removeRecursive(node->right, value, isRemoved);
    }
    else
    {
        isRemoved = true;
        Node* temp = node;

        if(!node->left || !node->right)
        {
            // Case 0 hoặc 1 con: Node sẽ "tan biến" hoặc bị thay thế bởi con
            std::string msg = node->left || node->right ? "Node has one child. Replacing..." : "Node is a leaf. Removing...";

            // Highlight node sắp đi vào dĩ vãng
            createSnapshot(node->id, msg, true);

            node = (node->left) ? node->left : node->right;
            delete temp;

            // Sau khi xóa, chúng ta return ngay vì không còn node này để balance tại đây
            return node;
        }
        else
        {
            // Case 2 con: Tìm successor (Node nhỏ nhất bên phải)
            Node* successor = findMin(node->right);

            // Nhịp 2: Highlight cả target và successor để người xem thấy sự liên hệ
            createSnapshot(successor->id, "Node has two children. Finding successor (min of right subtree): " + std::to_string(successor->data));

            // Nhịp 3: "Hóa thân" - Copy data nhưng giữ nguyên ID
            node->data = successor->data;
            createSnapshot(node->id, "Copying successor value " + std::to_string(node->data) + " to target node.");

            // Nhịp 4: Đệ quy xóa cái node successor thực sự ở dưới đáy
            removeRecursive(node->right, successor->data, isRemoved);
        }
    }

    // --- GIAI ĐOẠN UNWINDING (QUAY LUI) ---
    if(!node) return node;

    updateHeight(node);

    // Nhịp 5: Pulse tín hiệu từ dưới lên sau khi xóa
    // Ta lấy ID của con (nếu có) để tạo hiệu ứng Pulse truyền lên cha
    int childId = -1;
    if(value < node->data && node->left) childId = node->left->id;
    else if(value >= node->data && node->right) childId = node->right->id;

    createSnapshot(node->id, "Re-calculating height for " + std::to_string(node->data), false, -1);

    balance(node);

    return node;
}

    // --- CÂN BẰNG & XOAY ---

void AVLTree::balance(Node*& n)
{
    int bf = getBalanceFactor(n);

    if(std::abs(bf) > 1)
    {
        // Nhịp: Phát hiện vi phạm (Đổi màu Cam/Đỏ)
        createSnapshot(n->id, "Imbalance detected at node " + std::to_string(n->data) + " (BF = " + std::to_string(bf) + ")", true);

        if(bf > 1)
        {
            if(getBalanceFactor(n->left) < 0)
            {
                createSnapshot(n->left->id, "Double Rotation: Left rotation on child first", true);
                rotateLeft(n->left);
            }
            rotateRight(n);
        }
        else
        {
            if(getBalanceFactor(n->right) > 0)
            {
                createSnapshot(n->right->id, "Double Rotation: Right rotation on child first", true);
                rotateRight(n->right);
            }
            rotateLeft(n);
        }
    }
}

void AVLTree::rotateRight(Node*& y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;

    updateHeight(y);
    updateHeight(x);

    y = x; // Cực kỳ quan trọng: Gán lại tham chiếu để Node cha cập nhật ngay lập tức
    createSnapshot(y->id, "Right rotation completed.");
}

void AVLTree::rotateLeft(Node*& x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;

    updateHeight(x);
    updateHeight(y);

    x = y; // Cập nhật ngay lập tức vào cây
    createSnapshot(x->id, "Left rotation completed.");
}

    // --- HỆ THỐNG VẼ (SNAPSHOT & LAYOUT) ---

    // --- SNAPSHOT SYSTEM (Sử dụng trực tiếp ISnapshot) ---

void AVLTree::createSnapshot(int highlightNodeId, const std::string& message, bool isWarning, int pulseChildId)
{
    auto snap = std::make_shared<Core::ISnapshot>();
    // Gọi computeLayout bắt đầu từ Root (depth = 0)
    int totalDepth = getHeight(m_root);
    computeLayout(m_root, 960.f, m_startY, 0, totalDepth, snap->nodeStates, snap->edgeStates);

    // 1. Logic Highlight Node
    for(auto& node : snap->nodeStates)
    {
        if(node.id == highlightNodeId)
        {
            if(isWarning)
            {
                node.fillColor = sf::Color(255, 100, 0); // Warning Orange
                node.scale = 1.25f;
            }
            else
            {
                node.fillColor = sf::Color::Yellow; // Checking state
                node.scale = 1.1f;
            }
        }
    }

    // 2. Logic Pulse cho Edge (Tín hiệu truyền ngược)
    if(pulseChildId != -1 && highlightNodeId != -1)
    {
        for(auto& edge : snap->edgeStates)
        {
            // Tìm cạnh nối giữa node cha (highlight) và node con vừa đệ quy xong
            if(edge.startNodeId == highlightNodeId && edge.endNodeId == pulseChildId)
            {
                edge.isPulsing = true;
                edge.pulseColor = sf::Color::Cyan; // Màu tia điện
                // Để pulse chạy ngược từ con lên cha:
                edge.fillFromStart = false;
            }
        }
    }

    snap->logMessage = message;
    if(m_timeline) m_timeline->addSnapshot(snap);
}

void AVLTree::computeLayout(Node* node, float x, float y, int depth, int maxDepth,
                            std::vector<Core::NodeState>& nodes,
                            std::vector<Core::EdgeState>& edges)
{
    if(!node) return;

    // 1. TÍNH TOÁN KHOẢNG CÁCH DỌC ĐỘNG
    // Càng nhiều tầng, khoảng cách giữa mỗi tầng càng rộng ra
    float dynamicVGap = 100.0f + (maxDepth * 15.0f);

    // 2. TÍNH TOÁN KHOẢNG CÁCH NGANG DỰA TRÊN LÁ
    // K là khoảng cách hằng số giữa 2 node lá (ví dụ: 60px)
    const float K = 80.0f;
    // hOffset là khoảng cách từ cha đến con theo trục X
    float hOffset = (K / 2.0f) * std::pow(2.0f, maxDepth - depth - 1);

    // 3. THIẾT LẬP TRẠNG THÁI NODE
    Core::NodeState state;
    state.id = node->id;
    state.position = {x, y};
    state.value = std::to_string(node->data);
    state.transition = Core::TransitionType::ArcSwing;
    state.arcPivot = {960.f, -400.f}; // Pivot tổng quát

    nodes.push_back(state);

    // 4. ĐỆ QUY XUỐNG CON (Cha nằm trên đường trung trực của 2 con)
    if(node->left)
    {
        edges.push_back({node->id, node->left->id});
        // Node con trái dịch sang trái một khoảng hOffset
        computeLayout(node->left, x - hOffset, y + dynamicVGap, depth + 1, maxDepth, nodes, edges);
    }

    if(node->right)
    {
        edges.push_back({node->id, node->right->id});
        // Node con phải dịch sang phải một khoảng hOffset
        computeLayout(node->right, x + hOffset, y + dynamicVGap, depth + 1, maxDepth, nodes, edges);
    }
}

    // --- TIỆN ÍCH ---
    int AVLTree::getHeight(Node* n) { return n ? n->height : 0; }
    int AVLTree::getBalanceFactor(Node* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }
    void AVLTree::updateHeight(Node* n) { if(n) n->height = 1 + std::max(getHeight(n->left), getHeight(n->right)); }
    AVLTree::Node* AVLTree::findMin(Node* n) { while(n->left) n = n->left; return n; }
    void AVLTree::clearRecursive(Node* node) { if(!node) return; clearRecursive(node->left); clearRecursive(node->right); delete node; }

}
