#include "Heap.h"
#include "ResourceManager.h"
#include "DynamicIsland.h"
#include "AnimationUtils.h"
#include "TreeLayoutUtils.h"
#include <cmath>
#include <queue>
#include <algorithm>

namespace DS
{
    Heap::Heap()
    {
        m_font = &ResourceManager::getInstance().getFont("assets/fonts/font.ttf");
    }

    Heap::~Heap()
    {
        clear();
    }

    void Heap::bindDynamicIsland(GUI::DynamicIsland* island)
    {
        m_animManager.setDynamicIsland(island);
    }

    std::string Heap::getName() const
    {
        return "Max Heap";
    }

    std::vector<DS::Command> Heap::getCommands()
    {
        std::vector<DS::Command> cmds;

        cmds.push_back(DS::Command("Insert", DS::InputType::Integer, [this](DS::InputArgs args)
        {
            this->insert(args.iVal1);
        }));

        cmds.push_back(DS::Command("Extract Max", DS::InputType::None, [this](DS::InputArgs)
        {
            this->extractMax();
        }));

        cmds.push_back(DS::Command("Clear", DS::InputType::None, [this](DS::InputArgs)
        {
            this->clear();
        }));

        return cmds;
    }

    void Heap::insert(int value)
    {
        m_data.push_back(value);

        GUI::NodeUI* newNode = new GUI::NodeUI(m_font);
        newNode->setValue(std::to_string(value));
        newNode->setExactPosition(600.0f, 50.0f);
        m_nodes.push_back(newNode);

        Utils::AnimationStep step;
        step.logMessage = "Inserting value: " + std::to_string(value);
        step.duration = 1.0f;
        step.onStart = [this]()
        {
            this->updateLayout();
            this->rebuildEdges();
        };
        step.onFinish = [this]()
        {
            this->heapifyUp(this->m_data.size() - 1);
        };

        m_animManager.addStep(step);
        m_animManager.play();
    }

    void Heap::extractMax()
    {
        if(m_data.empty())
        {
            Utils::AnimationStep emptyStep;
            emptyStep.logMessage = "Heap is empty. Cannot extract max.";
            emptyStep.duration = 1.5f;
            m_animManager.addStep(emptyStep);
            m_animManager.play();
            return;
        }

        int maxValue = m_data[0];

        if(m_data.size() == 1)
        {
            Utils::AnimationStep singleStep;
            singleStep.logMessage = "Extracting the only value: " + std::to_string(maxValue);
            singleStep.duration = 1.5f;
            singleStep.onFinish = [this]()
            {
                this->clear();
            };
            m_animManager.addStep(singleStep);
            m_animManager.play();
            return;
        }

        Utils::AnimationStep highlightStep;
        highlightStep.logMessage = "Extracting max value: " + std::to_string(maxValue);
        highlightStep.duration = 1.5f;
        highlightStep.onStart = [this]()
        {
            this->m_nodes[0]->setTargetColor(sf::Color::Red, sf::Color::White, sf::Color::Red);
            this->m_nodes.back()->setTargetColor(sf::Color::Green, sf::Color::White, sf::Color::Green);
        };
        highlightStep.onFinish = [this]()
        {
            this->m_nodes[0]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
            this->m_nodes.back()->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));

            std::swap(this->m_data[0], this->m_data.back());
            std::swap(this->m_nodes[0], this->m_nodes.back());

            this->m_data.pop_back();
            GUI::NodeUI* rootNode = this->m_nodes.back();
            this->m_nodes.pop_back();
            delete rootNode;

            this->updateLayout();
            this->rebuildEdges();

            Utils::AnimationStep downStep;
            downStep.logMessage = "Heapifying down from root";
            downStep.duration = 0.5f;
            downStep.onFinish = [this]()
            {
                this->heapifyDown(0);
            };
            this->m_animManager.addStep(downStep);
        };

        m_animManager.addStep(highlightStep);
        m_animManager.play();
    }

    void Heap::clear()
    {
        Utils::AnimationStep clearStep;
        clearStep.logMessage = "Clearing the heap.";
        clearStep.duration = 0.5f;
        clearStep.onStart = [this]()
        {
            this->m_data.clear();

            for(auto node : this->m_nodes)
            {
                delete node;
            }
            this->m_nodes.clear();

            for(auto edge : this->m_edges)
            {
                delete edge;
            }
            this->m_edges.clear();
        };

        m_animManager.clear();
        m_animManager.addStep(clearStep);
        m_animManager.play();
    }


    void Heap::swapNodes(int i, int j)
    {
        std::swap(m_data[i], m_data[j]);
        std::swap(m_nodes[i], m_nodes[j]);

        GUI::NodeUI* node1 = m_nodes[j];
        GUI::NodeUI* node2 = m_nodes[i];

        auto onStartSwap = [this, node1, node2]()
        {
            for (auto it = this->m_edges.begin(); it != this->m_edges.end(); )
            {
                GUI::EdgeUI* edge = *it;
                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
                {
                    delete edge;
                    it = this->m_edges.erase(it);
                }
                else
                {
                    edge->setFocusState(false);
                    ++it;
                }
            }
        };

        auto onFinishSwap = [this, node1, node2]()
        {
            this->updateLayout();
            this->rebuildEdges();

            // Ẩn tất cả cạnh mới bằng màu tàng hình (Alpha = 0), đánh thức cạnh cũ
            for (auto edge : this->m_edges)
            {
                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
                {
                    // baseColor = Alpha 0, fillColor = Alpha 255
                    edge->setColorFilling(0.0f, sf::Color(150, 150, 150, 0), sf::Color(150, 150, 150, 255), true);
                }
                else
                {
                    edge->setFocusState(true);
                    edge->clearColorFilling();
                }
            }
        };

        Utils::AnimationStep swapStep = Utils::AnimationUtils::createOrbitalSwap(
            node1, node2, onStartSwap, onFinishSwap, 1.2f,
            "Swapping " + std::to_string(m_data[j]) + " and " + std::to_string(m_data[i])
        );
        m_animManager.addStep(swapStep);

        int n = m_data.size();
        std::vector<int> dist(n, 1e9);
        std::queue<int> q;

        for (int k = 0; k < n; ++k)
        {
            if (k != i && k != j)
            {
                dist[k] = 0;
                q.push(k);
            }
        }

        if (q.empty() && n > 0)
        {
            dist[0] = 0;
            q.push(0);
        }

        while (!q.empty())
        {
            int u = q.front(); q.pop();
            int parent = (u - 1) / 2;
            int left = 2 * u + 1;
            int right = 2 * u + 2;

            std::vector<int> neighbors;
            if (u > 0) neighbors.push_back(parent);
            if (left < n) neighbors.push_back(left);
            if (right < n) neighbors.push_back(right);

            for (int v : neighbors)
            {
                if (dist[v] > dist[u] + 1)
                {
                    dist[v] = dist[u] + 1;
                    q.push(v);
                }
            }
        }

        Utils::AnimationStep revealStep;
        revealStep.duration = 0.8f;
        revealStep.logMessage = "Rebuilding connections sequentially...";

        revealStep.onUpdate = [this, node1, node2, dist](float t)
        {
            for (auto edge : this->m_edges)
            {
                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
                {
                    int u = -1, v = -1;
                    for (int k = 0; k < this->m_nodes.size(); ++k)
                    {
                        if (this->m_nodes[k] == edge->getStartNode()) u = k;
                        if (this->m_nodes[k] == edge->getEndNode()) v = k;
                    }

                    if (u != -1 && v != -1)
                    {
                        int w = std::min(dist[u], dist[v]);
                        int max_w = 1;

                        float startTime = (float)w / (max_w + 1.0f);
                        float endTime = (float)(w + 1.0f) / (max_w + 1.0f);

                        float localProgress = (t - startTime) / (endTime - startTime);
                        localProgress = std::max(0.0f, std::min(1.0f, localProgress));

                        bool fromStart = (dist[u] <= dist[v]);

                        sf::Color transparent(150, 150, 150, 0);
                        sf::Color solid(150, 150, 150, 255);

                        // Cập nhật lan màu (Alpha)
                        edge->setColorFilling(localProgress, transparent, solid, fromStart);
                    }
                }
            }
        };

        revealStep.onFinish = [this, node1, node2]()
        {
            for (auto edge : this->m_edges)
            {
                edge->clearColorFilling();
            }
        };

        m_animManager.addStep(revealStep);
    }

//    void Heap::swapNodes(int i, int j)
//    {
//        std::swap(m_data[i], m_data[j]);
//        std::swap(m_nodes[i], m_nodes[j]);
//
////        Utils::AnimationStep step;
////        step.logMessage = "Swapping " + std::to_string(m_data[j]) + " and " + std::to_string(m_data[i]);
////        step.duration = 1.2f;
//
//        // Lấy sẵn 2 con trỏ để dùng trong lambda onUpdate cho an toàn
//        GUI::NodeUI* node1 = m_nodes[i];
//        GUI::NodeUI* node2 = m_nodes[j];
//
//        // Lấy vị trí gốc ngay tại thời điểm bắt đầu Swap
//        sf::Vector2f start1 = node1->getCurrentPosition();
//        sf::Vector2f start2 = node2->getCurrentPosition();
//
//        // ==========================================
//        // STEP 1: KỊCH BẢN BAY LƯỢN & LÀM MỜ (1.2s)
//        // ==========================================
//        auto onStartSwap = [this, node1, node2]()
//        {
//            // Duyệt ngược mảng cạnh để xóa an toàn
//            for (auto it = this->m_edges.begin(); it != this->m_edges.end(); )
//            {
//                GUI::EdgeUI* edge = *it;
//
//                // Nếu cạnh nối với 1 trong 2 Node đang xoay -> Cắt đứt (Xóa)
//                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
//                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
//                {
//                    delete edge;
//                    it = this->m_edges.erase(it);
//                }
//                else
//                {
//                    // Nếu là cạnh của các đỉnh không xoay -> Làm mờ và teo nhỏ
//                    edge->setFocusState(false);
//                    ++it;
//                }
//            }
//        };
//
//        auto onFinishSwap = [this]()
//        {
//            // Khi hạ cánh: Cập nhật tọa độ chuẩn và giăng lại toàn bộ dây
//            // Hàm rebuildEdges mặc định sẽ tạo dây mới với trạng thái Focus (Sáng rực)
//            this->updateLayout();
//            this->rebuildEdges();
//        };
//
//        Utils::AnimationStep swapStep = Utils::AnimationUtils::createOrbitalSwap(
//            node1, node2, onStartSwap, onFinishSwap, 1.2f,
//            "Swapping " + std::to_string(m_data[j]) + " and " + std::to_string(m_data[i])
//        );
//        m_animManager.addStep(swapStep);
//
//        // ==========================================
//        // STEP 2: KỊCH BẢN LAN TIA SÁNG RECONNECT (0.4s)
//        // ==========================================
//        Utils::AnimationStep pulseStep;
//        pulseStep.duration = 0.4f; // Tia sáng bắn rất nhanh và dứt khoát
//        pulseStep.logMessage = "Reconnecting links...";
//
//        pulseStep.onUpdate = [this, node1, node2](float t)
//        {
//            // Tìm các cạnh MỚI vừa được nối với node1 và node2 để bắn Pulse
//            for (auto edge : this->m_edges)
//            {
//                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
//                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
//                {
//                    // Bắn tia sáng màu Cyan (Xanh lơ) chạy từ Start đến End (Từ Cha xuống Con)
//                    edge->setPulse(t, sf::Color::Cyan);
//                }
//            }
//        };
//
//        pulseStep.onFinish = [this, node1, node2]()
//        {
//            // Tắt hiệu ứng hạt sáng khi đã chạy tới nơi
//            for (auto edge : this->m_edges)
//            {
//                if (edge->getStartNode() == node1 || edge->getEndNode() == node1 ||
//                    edge->getStartNode() == node2 || edge->getEndNode() == node2)
//                {
//                    edge->stopPulse();
//                }
//            }
//        };
//
//        m_animManager.addStep(pulseStep);


//
//
//        auto onStart = [this]()
//        {
////            this->rebuildEdges();
//            // 1. NGẮT KẾT NỐI: Xóa toàn bộ cạnh để tránh "Spaghetti Edges"
//            for(auto edge : this->m_edges)
//            {
//                delete edge;
//            }
//            this->m_edges.clear();
//        };
//
//        auto onFinish = [this]()
//        {
//            this->updateLayout();
//            this->rebuildEdges();
//        };
//
//        Utils::AnimationStep step = Utils::AnimationUtils::createOrbitalSwap(
//            node1,
//            node2,
//            onStart,
//            onFinish,
//            1.2f,
//            "Swapping " + std::to_string(m_data[j]) + " and " + std::to_string(m_data[i])
//        );
//
//        m_animManager.addStep(step);

//        step.onStart = [this, i, j]()
//        {
//            this->m_nodes[i]->setTargetColor(sf::Color::Red, sf::Color::White, sf::Color::Red);
//            this->m_nodes[j]->setTargetColor(sf::Color::Red, sf::Color::White, sf::Color::Red);
//            this->updateLayout();
//            this->rebuildEdges();
//        };
//
//        step.onUpdate = [node1, node2, start1, start2](float t)
//        {
//            const float PI = 3.14159265f;
//
//            // 1. Ép thời gian t thành dạng Ease-In-Out (Chậm -> Nhanh -> Chậm)
//            float easedT = (1.0f - std::cos(t * PI)) / 2.0f;
//
//            sf::Vector2f dir = start2 - start1;
//            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
//
//            sf::Vector2f normal(0.0f, 0.0f);
//            if(len > 0.0001f)
//            {
//                normal = sf::Vector2f(-dir.y / len, dir.x / len);
//            }
//
//            float arcHeight = std::min(len * 0.3f, 150.0f);
//
//            // 2. Độ cong của vòng cung cũng nương theo thời gian đã làm mượt
//            float curve = std::sin(easedT * PI) * 1.3;
//
//            // 3. Nội suy vị trí theo easedT thay vì t tuyến tính
//            sf::Vector2f current1 = start1 + (start2 - start1) * easedT + normal * (curve * arcHeight);
//            sf::Vector2f current2 = start2 + (start1 - start2) * easedT + normal * (-curve * arcHeight);
//
//            node1->setExactPosition(current1.x, current1.y);
//            node2->setExactPosition(current2.x, current2.y);
//        };
//
//        step.onFinish = [this, i, j, node1, node2]()
//        {
//            // Khóa mục tiêu: Đảm bảo khi kết thúc, độ lệch trở về 0 hoàn hảo
//            node1->setArcOffset({0.0f, 0.0f});
//            node2->setArcOffset({0.0f, 0.0f});
//
//            this->m_nodes[i]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
//            this->m_nodes[j]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
//        };
//
//        m_animManager.addStep(step);
//    }

    void Heap::heapifyUp(int index)
    {
        if(index <= 0)
        {
            Utils::AnimationStep doneStep;
            doneStep.logMessage = "Heapify up complete.";
            doneStep.duration = 1.0f;
            m_animManager.addStep(doneStep);
            return;
        }

        int parent = (index - 1) / 2;

        Utils::AnimationStep compareStep;
        compareStep.logMessage = "Comparing Node " + std::to_string(m_data[index]) + " with Parent " + std::to_string(m_data[parent]);
        compareStep.duration = 1.0f;
        compareStep.onStart = [this, index, parent]()
        {
            this->m_nodes[index]->setTargetColor(sf::Color::Yellow, sf::Color::Black, sf::Color::Yellow);
            this->m_nodes[parent]->setTargetColor(sf::Color::Yellow, sf::Color::Black, sf::Color::Yellow);
        };
        compareStep.onFinish = [this, index, parent]()
        {
            this->m_nodes[index]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
            this->m_nodes[parent]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));

            if(this->m_data[index] > this->m_data[parent])
            {
                this->swapNodes(index, parent);

                Utils::AnimationStep recurseStep;
                recurseStep.duration = 0.1f;
                recurseStep.onFinish = [this, parent]()
                {
                    this->heapifyUp(parent);
                };
                this->m_animManager.addStep(recurseStep);
            }
            else
            {
                Utils::AnimationStep doneStep;
                doneStep.logMessage = "Node is correctly placed.";
                doneStep.duration = 1.0f;
                this->m_animManager.addStep(doneStep);
            }
        };

        m_animManager.addStep(compareStep);
    }

    void Heap::heapifyDown(int index)
    {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        std::vector<int> nodesToHighlight = {index};
        if(left < m_data.size()) nodesToHighlight.push_back(left);
        if(right < m_data.size()) nodesToHighlight.push_back(right);

        if(nodesToHighlight.size() == 1)
        {
            Utils::AnimationStep doneStep;
            doneStep.logMessage = "Heapify down complete.";
            doneStep.duration = 1.0f;
            m_animManager.addStep(doneStep);
            return;
        }

        Utils::AnimationStep compareStep;
        compareStep.logMessage = "Finding largest among parent and children";
        compareStep.duration = 1.5f;
        compareStep.onStart = [this, nodesToHighlight]()
        {
            for(int idx : nodesToHighlight)
            {
                this->m_nodes[idx]->setTargetColor(sf::Color::Yellow, sf::Color::Black, sf::Color::Yellow);
            }
        };
        compareStep.onFinish = [this, index, left, right, largest, nodesToHighlight]() mutable
        {
            for(int idx : nodesToHighlight)
            {
                this->m_nodes[idx]->setTargetColor(sf::Color::White, sf::Color::Black, sf::Color(200, 200, 200));
            }

            if(left < this->m_data.size() && this->m_data[left] > this->m_data[largest])
            {
                largest = left;
            }

            if(right < this->m_data.size() && this->m_data[right] > this->m_data[largest])
            {
                largest = right;
            }

            if(largest != index)
            {
                this->swapNodes(index, largest);

                Utils::AnimationStep recurseStep;
                recurseStep.duration = 0.1f;
                recurseStep.onFinish = [this, largest]()
                {
                    this->heapifyDown(largest);
                };
                this->m_animManager.addStep(recurseStep);
            }
            else
            {
                Utils::AnimationStep doneStep;
                doneStep.logMessage = "Heapify down complete.";
                doneStep.duration = 1.0f;
                this->m_animManager.addStep(doneStep);
            }
        };

        m_animManager.addStep(compareStep);
    }

//    void Heap::updateLayout()
//    {
//        if(m_nodes.empty()) return;
//
//        float startX = 600.0f;
//        float startY = 150.0f;
//        float verticalGap = 100.0f;
//        float horizontalGap = 400.0f;
//
//        for(int i = 0; i < m_nodes.size(); ++i)
//        {
//            int level = std::log2(i + 1);
//            int indexInLevel = i - ((1 << level) - 1);
//            int nodesInLevel = 1 << level;
//
//            float currentHGap = horizontalGap / (1 << level);
//            float x = startX - (nodesInLevel - 1) * currentHGap / 2.0f + indexInLevel * currentHGap;
//            float y = startY + level * verticalGap;
//
//            m_nodes[i]->setTargetPosition(x, y);
//        }
//    }

    void Heap::updateLayout()
    {
        if (m_nodes.empty())
        {
            return;
        }

        // Vị trí của Root (bạn có thể thay đổi số 1920 tùy theo chiều rộng màn hình)
        sf::Vector2f rootPosition(1920.0f / 2.0f, 100.0f);

        // Gọi hàm tính toán (60.0f là khoảng cách tối thiểu giữa 2 lá, 80.0f là chiều cao mỗi tầng)
        Utils::TreeLayoutUtils::updateHeapLayout(m_nodes, rootPosition, 60.0f, 80.0f);
    }

    void Heap::rebuildEdges()
    {
        for(auto edge : m_edges)
        {
            delete edge;
        }
        m_edges.clear();

        for(int i = 0; i < m_nodes.size(); ++i)
        {
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if(left < m_nodes.size())
            {
                GUI::EdgeUI* edgeL = new GUI::EdgeUI(m_nodes[i], m_nodes[left], 10.0f);
                m_edges.push_back(edgeL);
            }
            if(right < m_nodes.size())
            {
                GUI::EdgeUI* edgeR = new GUI::EdgeUI(m_nodes[i], m_nodes[right], -10.0f);
                m_edges.push_back(edgeR);
            }
        }
    }

    void Heap::update(float dt)
    {
        m_animManager.update(dt);

        for(auto node : m_nodes)
        {
            node->update(dt);
        }

        for(auto edge : m_edges)
        {
            edge->update(dt);
        }
    }

    void Heap::draw(sf::RenderTarget& target)
    {
        for(auto edge : m_edges)
        {
            edge->draw(target);
        }

        for(auto node : m_nodes)
        {
            node->draw(target);
        }
    }

    // Trong Heap.cpp
    void Heap::setMousePosition(sf::Vector2f pos)
    {
        for(auto edge : m_edges)
        {
            edge->setMousePosition(pos);
        }
    }
}
