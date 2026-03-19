#pragma once
#include "DataStructure.h"
#include "GUI/NodeUI.h"
#include "GUI/EdgeUI.h"
#include "AnimationManager.h"

namespace DS
{

class Heap : public DataStructure
{
private:
    std::vector<int> m_data;                   // Dữ liệu logic cốt lõi
    std::vector<GUI::NodeUI*> m_nodes;         // Các Node vật lý hiển thị trên màn hình
    std::vector<GUI::EdgeUI*> m_edges;         // Các Cạnh nối đàn hồi (Bezier)

    Utils::AnimationManager m_animManager;     // Quản lý các hoạt ảnh sinh ra từ thao tác
    sf::Font* m_font;

    // --- CÁC HÀM HỖ TRỢ THUẬT TOÁN & HOẠT ẢNH ---
    void heapifyUp(int index);
    void heapifyDown(int index);

    // Tính toán tọa độ đích (target) cho toàn bộ Node dựa vào index (Vẽ dạng cây)
    void updateLayout();

    // Hàm hoán đổi 2 phần tử: Đổi logic mảng, đổi con trỏ Node, và nạp Animation
    void swapNodes(int i, int j);

    // Tạo lại toàn bộ các EdgeUI kết nối cha - con (i -> 2i+1, 2i+2)
    void rebuildEdges();

public:
    Heap();
    ~Heap() override;

    // --- CÁC THAO TÁC CỦA HEAP ---
    void insert(int value);
    void extractMax();
    void clear();

    // --- IMPLEMENT TỪ DATASTRUCTURE ---
    std::vector<DS::Command> getCommands() override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    std::string getName() const override;

    // Trong Heap.h
    void bindDynamicIsland(GUI::DynamicIsland* island) override;

    void setMousePosition(sf::Vector2f pos) override;
};

}
