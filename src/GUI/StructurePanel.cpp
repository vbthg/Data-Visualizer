#include "StructurePanel.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "ViewHandler.h"
#include <iostream>
#include <cmath>
#include <unordered_set>

/*
        float sdSuperellipse(vec2 p, vec2 b, float r, float power) {
            vec2 q = abs(p) - b + r;
            vec2 maxQ = max(q, 0.0);
            float distN = pow(pow(maxQ.x, power) + pow(maxQ.y, power), 1.0 / power);
            return min(max(q.x, q.y), 0.0) + distN - r;
        }
*/

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    // Sửa lại đoạn shader trong maskShaderCode
    // Sửa lại đoạn shader trong maskShaderCode
    const std::string maskShaderCode = R"(
#version 120
uniform sampler2D u_texture;
uniform vec2 u_halfSize;    // Truyền từ CPU: u_panelSize * 0.5
uniform vec2 u_innerBound;  // Truyền từ CPU: u_halfSize - vec2(u_radius)
uniform float u_radius;

void main() {
    vec2 uv = gl_TexCoord[0].xy;

    // Tính tọa độ pixel so với tâm (Local Position)
    // u_halfSize * 2.0 chính là u_panelSize ban đầu
    vec2 p = (uv * (u_halfSize * 2.0)) - u_halfSize;
    vec2 ap = abs(p);

    // --- TỐI ƯU 1: EARLY EXIT (Vùng chữ thập) ---
    if (ap.x < u_innerBound.x || ap.y < u_innerBound.y) {
        gl_FragColor = texture2D(u_texture, uv);
        return;
    }

    // --- TỐI ƯU 2: CHỈ TÍNH TOÁN CHO 4 GÓC ---
    vec2 q = ap - u_innerBound;

    // Kiểm tra nhanh: Nếu pixel nằm quá xa bán kính (góc ngoài cùng), hủy pixel luôn
    // Điều này giúp tiết kiệm việc texture2D cho những vùng chắc chắn trong suốt
    if (q.x > u_radius + 0.5 && q.y > u_radius + 0.5) {
        discard;
    }

    // Tối ưu n=4 (không dùng pow)
    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float dist = sqrt(sqrt(x2 * x2 + y2 * y2)) - u_radius;

    // Khử răng cưa
    float alphaMask = 1.0 - smoothstep(-0.5, 0.5, dist);

    // Nếu hoàn toàn trong suốt thì không cần sample texture
    if (alphaMask <= 0.0) {
        discard;
    }

    vec4 color = texture2D(u_texture, uv);
    gl_FragColor = vec4(color.rgb, color.a * alphaMask);
}
    )";

    StructurePanel::StructurePanel()
    {
        m_isPanning = false;
        m_padding = 24.0f;
        m_codePanelWidth = 450.0f;
        m_dockSpace = 150.0f;

        m_frame = new GUI::Squircle({600.f, 600.f});
        m_frame->setRadius(30.f);
        m_frame->setOutlineThickness(1.5f);
        m_frame->setBakedGlass(&ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 50.png"), sf::Vector2f(1920.f, 1080.f));
        m_frame->setPosition(m_padding, m_padding);
//        m_frame->setFillColor(sf::Color::White, 0.3f);
        m_frame->setShadow(Theme::Color::DockShadow, 55.f, {0.f, 20.f});

        m_widthSpring.stiffness = 400.f; m_widthSpring.damping = 40.f;
        m_heightSpring.stiffness = 400.f; m_heightSpring.damping = 40.f;
        m_alphaSpring.stiffness = 300.f; m_alphaSpring.damping = 35.f;

        m_zoomSpring.stiffness = 300.0f; // Thấp hơn một chút để tạo cảm giác "mềm"
        m_zoomSpring.damping = 30.0f;
        m_zoomSpring.snapTo(1.0f); // Mặc định là zoom 1:1

        m_viewCenterSpring.stiffness = 450.0f; // Thấp để trượt êm
        m_viewCenterSpring.damping = 60.0f;    // Ma sát vừa phải để có độ trượt dài
        m_viewCenterSpring.snapTo({960.f, 540.f});

        m_view.setSize(1920.f, 1080.f);
        m_view.setCenter(0.0f, 450.0f);
        m_viewCenterSpring.snapTo(sf::Vector2f(0.f, 450.f));

        // Trong StructurePanel constructor
        m_contentBuffer.create(1920, 1080); // Kích thước cố định lớn nhất
        m_contentBuffer.setSmooth(true);
//        m_view.setCenter(1920.f / 2.0f, 1080.f / 2.0f);

        // Load Shader cắt góc riêng biệt
        m_isMaskLoaded = sf::Shader::isAvailable() && m_maskShader.loadFromMemory(maskShaderCode, sf::Shader::Fragment);



        // 1. Create the button (assuming you have a font loaded)
        resetBtn = new GUI::Button(ResourceManager::getInstance().getFont("assets/fonts/Phosphor.ttf"), L"\uE626", {60.f, 60.f});
        resetBtn->applyPreset(GUI::ButtonPreset::Ghost);
        resetBtn->setCharacterSize(27);
        resetBtn->setPosition(sf::Vector2f(1.5f * m_padding + 30.f, 1.5f * m_padding + 30.f));

        // 2. Define the callback
        // We use a lambda to call the resetView method of your structurePanel instance
        resetBtn->onClick = [this]()
        {
            m_isAutoFollow = true;
            fitView(false);

            // Visual feedback for the button text
//            resetBtn->triggerTextPop();
        };


        m_emptyMessage.setFont(ResourceManager::getInstance().getFont("assets/fonts/SFProDisplay-Bold.ttf"));
        m_emptyMessage.setString("Start with a node.");
        m_emptyMessage.setCharacterSize(60);
        m_emptyMessage.setFillColor(sf::Color(255, 255, 255, 130)); // Trắng mờ (Muted)
        m_emptyMessage.setOutlineThickness(0.5f);
        m_emptyMessage.setOutlineColor(sf::Color(0, 0, 0, 50));

        m_isAutoFollow = true; // Thêm dòng này để Camera tự động chạy từ đầu
        m_isPanning = false;
//        m_needsReposition = true; // Cho phép tính toán ngay frame đầu tiên
    }

    StructurePanel::~StructurePanel()
    {
        if (m_frame) delete m_frame;
    }

    void StructurePanel::initIntro(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen)
    {
        updateLayout(windowWidth, windowHeight, isCodePanelOpen);
        m_widthSpring.position = m_widthSpring.target * 0.8f;
        m_heightSpring.position = m_heightSpring.target * 0.8f;
        m_alphaSpring.position = 0.0f;

        // Đảm bảo các node nằm ngay giữa panel khi vừa xuất hiện
//        fitView(true);
    }

    void StructurePanel::updateLayout(unsigned int windowWidth, unsigned int windowHeight, bool isCodePanelOpen)
    {
        m_heightSpring.target = windowHeight - (m_padding * 2.0f) - m_dockSpace;
        m_widthSpring.target = isCodePanelOpen ? (windowWidth - m_codePanelWidth - m_padding * 3.0f) : (windowWidth - m_padding * 2.0f);
        m_alphaSpring.target = 1.0f;

        // Gọi fitView để tính lại target Center dựa trên kích thước khung mới
        m_isAutoFollow = true;
        fitView(false);
    }

    sf::Vector2f StructurePanel::mapPanelPixelToWorld(sf::Vector2i mousePos, const sf::RenderWindow& window, float zoom)
    {
        // Nếu không truyền zoom cụ thể, lấy giá trị hiện tại của lò xo
        float currentZoom = (zoom < 0.f) ? m_zoomSpring.position : zoom;

        // 1. Chuyển từ pixel cửa sổ sang tọa độ UI (đã tính đến việc resize cửa sổ app)
        sf::Vector2f windowCoords = window.mapPixelToCoords(mousePos);

        // 2. Lấy Bounds của Frame để tính độ lệch (Local Offset)
        sf::FloatRect bounds = m_frame->getGlobalBounds();
        sf::Vector2f localOffset = windowCoords - sf::Vector2f(bounds.left, bounds.top);

        // 3. Tính toán hệ số Zoom dựa trên Buffer cố định (1920)
        // Hệ số này xác định 1 pixel trên Frame bằng bao nhiêu đơn  vị trong World
        float worldUnitsPerPixel = currentZoom;
        // Tính kích thước View tương ứng với mức zoom được yêu cầu
        sf::Vector2f viewSize(1920.0f * currentZoom, 1080.0f * currentZoom);

        // 4. Tìm góc trái trên của View (điểm này luôn khớp với pixel 0,0 của Buffer)
        sf::Vector2f viewTopLeft = m_viewCenterSpring.position - (viewSize / 2.0f);

        // 5. Công thức cuối cùng cho hệ Crop
        return viewTopLeft + (localOffset * worldUnitsPerPixel);
    }

    void StructurePanel::syncTimeline(Core::TimelineManager* timeline)
    {
        m_timeline = timeline;
    }

    void StructurePanel::syncDataStructure(DS::DataStructure* dsa)
    {
        m_dataStructure = dsa;
    }

    void StructurePanel::update(float dt, sf::RenderWindow& window)
    {
        resetBtn->update(window, dt);


        // 1. Logic Empty State & Alpha
        if(m_nodeUIMap.empty())
        {
            m_alphaSpring.target = 0.0f;

            // Căn giữa Origin cho văn bản nhiều dòng
            sf::FloatRect bounds = m_emptyMessage.getLocalBounds();
            // SFML mặc định căn lề trái, nên ta đặt tâm vào giữa bounds
            m_emptyMessage.setOrigin(bounds.left + bounds.width/2.0f,
                                     bounds.top + bounds.height/2.0f);

            sf::Vector2f center = m_frame->getPosition() + (m_frame->getSize() / 2.0f);
            m_emptyMessage.setPosition(center);
        }
        else
        {
//            m_alphaSpring.snapTo(1.f);
            m_alphaSpring.target = 1.f; // Hiện panel mờ khi có node
        }


        // Cập nhật lò xo camera
        m_viewCenterSpring.update(dt);
        m_view.setCenter(m_viewCenterSpring.position);

        m_widthSpring.update(dt);
        m_heightSpring.update(dt);
        m_alphaSpring.update(dt);


//        // TÍNH TOÁN VỊ TRÍ CHUỘT TRONG WORLD CỦA PANEL
//        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
//        sf::Vector2f mouseWorldPos = mapPanelPixelToWorld(mousePosI, window);





        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect bounds = getGlobalBounds();

//        if(bounds.contains(window.mapPixelToCoords(mousePos)))
        if(Utils::ViewHandler::isMouseInFrame(mousePos, window, bounds))
        {
            m_mouseWorldPos = mapPanelPixelToWorld(mousePos, window);
        }
        else
        {
            // NẾU RA NGOÀI BIÊN:
            // 1. Ép tất cả các Node đang dragging phải dừng lại (Release)
            for(auto& pair : m_nodeUIMap)
            {
                if(pair.second->isDragging())
                {
                    pair.second->forceRelease();

                    // Đồng bộ lại lịch sử và logic ngay khi bị ép thả
                    int nodeId = pair.first;
                    sf::Vector2f finalPos = pair.second->getCurrentPosition();
                    m_timeline->updateNodePositionInHistory(nodeId, finalPos);
                    if(m_dataStructure) m_dataStructure->updateNodePosition(nodeId, finalPos);
                }
            }

            // Nếu chuột văng ra ngoài panel, set một tọa độ cực xa để các Edge không bị hút
            m_mouseWorldPos = sf::Vector2f(-99999.0f, -99999.0f);
        }


        // 2. Cập nhật TẤT CẢ các Node đang có trong Map
        for(auto& pair : m_nodeUIMap)
        {
            // Truyền window vào để Node tự lấy vị trí chuột nếu đang Drag
            pair.second->update(dt, m_mouseWorldPos);
        }

        // 3. Cập nhật TẤT CẢ các Edge
        for(auto& pair : m_edgeUIMap)
        {
            pair.second->update(dt);
        }


        // 2. Logic Zoom mượt mà
        float oldZoom = m_zoomSpring.position;
        m_zoomSpring.update(dt);

        // 2. Nếu có sự thay đổi Zoom, thực hiện bù trừ tâm (Anchor)
        if(!m_isAutoFollow && std::abs(m_zoomSpring.position - oldZoom) > 0.0001f)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::FloatRect bounds = getGlobalBounds();

            if(bounds.contains(window.mapPixelToCoords(mousePos)))
            {
                // Dùng hàm map chuyên dụng cho Crop đã thống nhất ở turn trước
                // mouseWorldBefore tính dựa trên zoom cũ
                sf::Vector2f mouseWorldBefore = mapPanelPixelToWorld(mousePos, window, oldZoom);

                // Cập nhật size view mới
                m_view.setSize(1920.0f * m_zoomSpring.position, 1080.0f * m_zoomSpring.position);

                // mouseWorldAfter tính dựa trên zoom mới
                sf::Vector2f mouseWorldAfter = mapPanelPixelToWorld(mousePos, window, m_zoomSpring.position);

                // Cập nhật lò xo Center: Dịch chuyển cả Target và Position để không bị khựng
                sf::Vector2f offset = mouseWorldBefore - mouseWorldAfter;
                m_viewCenterSpring.target += offset;
                m_viewCenterSpring.position += offset;
            }
            else
            {
                m_view.setSize(1920.0f * m_zoomSpring.position, 1080.0f * m_zoomSpring.position);
            }
        }

        else if(m_isAutoFollow && std::abs(m_zoomSpring.position - oldZoom) > 0.0001f)
        {
            // Nếu đang Auto-Follow, ta vẫn cần cập nhật size của View theo lò xo zoom
            // nhưng KHÔNG cộng thêm offset bù trừ vào target Center
            m_view.setSize(1920.0f * m_zoomSpring.position, 1080.0f * m_zoomSpring.position);
        }



        float w = std::max(1.0f, m_widthSpring.position);
        float h = std::max(1.0f, m_heightSpring.position);
        m_frame->setSize({w, h});

        float alphaValue = std::max(0.0f, std::min(m_alphaSpring.position, 1.0f));
//        std::cout << alphaValue << "\n";
        m_frame->setFillColor(sf::Color::White, 0.2f * alphaValue);
//        m_frame->setFillColor(sf::Color::White, 0.2f * alphaValue);
        m_frame->setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(40 * alphaValue)));
//
//        if(std::abs((float)m_contentBuffer.getSize().x - w) > 2.0f
//        || std::abs((float)m_contentBuffer.getSize().y - h) > 2.0f)
//        {
//            m_contentBuffer.create((unsigned int)w, (unsigned int)h);
//            m_contentBuffer.setSmooth(true);
//
//            // QUAN TRỌNG: Cập nhật lại kích thước View để khớp với Buffer mới
//            // Nếu không, vật thể sẽ bị scale sai tỉ lệ
//            m_view.setSize(w, h);
//        }
    }

    void StructurePanel::handleEvent(const sf::Event& event, sf::RenderWindow& window)
    {
        resetBtn->handleEvent(event, window);

        if(m_nodeUIMap.empty()) return;

        // TÍNH TOÁN VỊ TRÍ CHUỘT TRONG WORLD CỦA PANEL
//        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
//        sf::Vector2f mouseWorldPos = mapPanelPixelToWorld(mousePosI, window);

        bool eventHandled = false;

        // Duyệt qua các node (Nên duyệt ngược để node nào vẽ sau/nằm trên sẽ được ưu tiên)
        for(auto& pair : m_nodeUIMap)
        {
            bool wasDragging = pair.second->isDragging();

            if(event.type == sf::Event::MouseButtonPressed)
            {
                // Nếu sự kiện chưa bị ai "nuốt", mới cho node tiếp theo thử
                if(!eventHandled)
                {
                    if(pair.second->handleEvent(event, m_mouseWorldPos))
                    {
                        eventHandled = true;
                        m_isAutoFollow = false;
                    }
                }
            }
            else
            {
                // Các sự kiện khác như Released cần được gửi cho tất cả
                // để đảm bảo các node đang drag đều được thả ra
                pair.second->handleEvent(event, m_mouseWorldPos);
            }

            // Đồng bộ dữ liệu khi thả chuột
            if(wasDragging && !pair.second->isDragging())
            {
                int nodeId = pair.first;
                sf::Vector2f finalPos = pair.second->getCurrentPosition();
                m_timeline->updateNodePositionInHistory(nodeId, finalPos);
                if(m_dataStructure) m_dataStructure->updateNodePosition(nodeId, finalPos);
            }
        }

        if(event.type == sf::Event::MouseWheelScrolled || m_isPanning)
        {
            m_isAutoFollow = false; // "Tôi tự làm được, đừng giành camera của tôi!"
        }

        // Bắt đầu Pan khi nhấn Space
        if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
        {
            m_isPanning = true;
            m_lastMousePos = sf::Mouse::getPosition(window);
        }

        // Kết thúc Pan khi nhả Space
        if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space)
        {
            m_isPanning = false;
        }

        // Thực hiện Pan khi chuột di chuyển và đang giữ Space
        if(event.type == sf::Event::MouseMoved && m_isPanning)
        {
            sf::Vector2i currentMousePos = {event.mouseMove.x, event.mouseMove.y};

            // Tính độ lệch Pixel
            sf::Vector2f delta;
            delta.x = static_cast<float>(m_lastMousePos.x - currentMousePos.x);
            delta.y = static_cast<float>(m_lastMousePos.y - currentMousePos.y);

            // Quy đổi delta pixel sang delta World:
            // Trong hệ Crop: 1 pixel = (m_view.size / 1920) units.
            float worldScale = m_zoomSpring.position; // Vì size = 1920 * zoom

            m_viewCenterSpring.target += delta * worldScale;
            m_lastMousePos = currentMousePos;
        }

        if(event.type == sf::Event::MouseWheelScrolled)
        {
            // Vuốt lên (delta > 0) -> Phóng to (giảm hệ số zoom)
            // Vuốt xuống (delta < 0) -> Thu nhỏ (tăng hệ số zoom)
            float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.15f;

            m_zoomSpring.target *= factor;

            // Giới hạn zoom từ 0.2x đến 5.0x để tránh lỗi hiển thị
            m_zoomSpring.target = std::max(0.6f, std::min(m_zoomSpring.target, 4.f));
        }
    }

//    void StructurePanel::renderContent(DataStructure* ds)
//    {
//        if (!ds || m_contentBuffer.getSize().x == 0) return;
//
//        m_contentBuffer.clear(sf::Color::Transparent);
//        m_contentBuffer.setView(m_view);
//        ds->draw(m_contentBuffer);
//        m_contentBuffer.display();
//    }

    // StructurePanel.cpp

//    void StructurePanel::renderContent(DataStructure* ds, const Core::RenderFrame& frame)
//    {
////        std::cout << "DEBUG: RenderTexture Created (" << w << "x" << h << ")" << std::endl;
////        std::cout << "DEBUG: RenderTexture Created (" << std::endl;
//        // Kiểm tra an toàn
//        if (!ds || m_contentBuffer.getSize().x == 0) return;
//
//        m_contentBuffer.clear(sf::Color::Transparent);
//
//        // Áp dụng Camera (Pan/Zoom) hiện tại của Panel vào Buffer
//        m_contentBuffer.setView(m_view);
//
//        // THAY ĐỔI QUAN TRỌNG:
//        // Thay vì ds->draw(m_contentBuffer), ta gọi hàm render mới
//        // Hàm này sẽ vẽ các Node/Edge theo tọa độ đã được nội suy trong frame
//        ds->render(m_contentBuffer, frame);
//
//        m_contentBuffer.display();
//    }

void StructurePanel::syncGraphObjects(const Core::RenderFrame& frame, float dt)
{
    // --- 1. CHUẨN BỊ TRA CỨU (O(N)) ---
    // Chuyển sang unordered_set để việc tìm kiếm đạt O(1)
    std::unordered_set<int> currentFrameNodes;
    std::unordered_set<uint64_t> currentFrameEdges;

//    std::cout << "SyncGraphObject Called!\n";

    // --- 2. ĐỒNG BỘ NODE ---
    for(const auto& nodeState : frame.nodes)
    {
//        std::cout << "NODE IN SYNCGRAPHOBJECT\n";
        currentFrameNodes.insert(nodeState.id);

        if(m_nodeUIMap.find(nodeState.id) == m_nodeUIMap.end())
        {
            float defaultRadius = 25.0f;
            m_nodeUIMap[nodeState.id] = std::make_unique<NodeUI>(
                &ResourceManager::getInstance().getFont("assets/fonts/SFProtext-regular.ttf"),
                defaultRadius,
                &nodeState);

            // Có node mới -> Cần tính lại vị trí
            m_needsReposition = true;

//            std::cout << "[NODE ID]: " << nodeState.id << "\n";
        }

        auto& nodeUI = m_nodeUIMap[nodeState.id];
        // CHỈ applyState nếu Node KHÔNG ở trạng thái đang bị kéo
        if(!nodeUI->isDragging())
        {
            nodeUI->applyState(nodeState);
        }
        else
        {
            // TÙY CHỌN: Nếu bạn muốn Core biết vị trí mới của Node sau khi kéo,
            // bạn có thể gửi tọa độ nodeUI->getCurrentPosition() ngược lại cho Core tại đây.
        }
//        nodeUI->update(dt);

        // Kiểm tra vận tốc: chỉ cần kiểm tra trị tuyệt đối
        sf::Vector2f vel = nodeUI->getVelocity();
        if(std::abs(vel.x) > 0.1f || std::abs(vel.y) > 0.1f)
            m_needsReposition = true;
    }

    // --- 3. ĐỒNG BỘ EDGE ---
    for(const auto& edgeState : frame.edges)
    {
//        std::cout << "EDGE IN SYNCGRAPHOBJECT\n";

        uint64_t edgeKey = getEdgeKey(edgeState.startNodeId, edgeState.endNodeId);
        currentFrameEdges.insert(edgeKey);

        if(m_edgeUIMap.find(edgeKey) == m_edgeUIMap.end())
        {
            NodeUI* startNode = m_nodeUIMap[edgeState.startNodeId].get();
            NodeUI* endNode = m_nodeUIMap[edgeState.endNodeId].get();
            m_edgeUIMap[edgeKey] = std::make_unique<EdgeUI>(startNode, endNode);

            m_needsReposition = true;
        }

        auto& edgeUI = m_edgeUIMap[edgeKey];
        edgeUI->setMousePosition(m_mouseWorldPos);
        edgeUI->applyState(edgeState);
        edgeUI->update(dt);
    }

    // --- 4. DỌN DẸP RÁC (Garbage Collection - O(N)) ---
    // Xóa Node cũ
    for(auto it = m_nodeUIMap.begin(); it != m_nodeUIMap.end();)
    {
        if(currentFrameNodes.find(it->first) == currentFrameNodes.end()) {
            it = m_nodeUIMap.erase(it);
            m_needsReposition = true; // Xóa node cũng cần tính lại BBox
        } else {
            ++it;
        }
    }

    // Xóa Edge cũ
    for(auto it = m_edgeUIMap.begin(); it != m_edgeUIMap.end();)
    {
        if(currentFrameEdges.find(it->first) == currentFrameEdges.end())
        {
            it = m_edgeUIMap.erase(it);
            m_needsReposition = true;
        }
        else
        {
            ++it;
        }
    }

//    std::cout << m_isAutoFollow << " " << m_needsReposition << " " << m_isPanning << "\n";

    // --- 5. SMART FIT VIEW (Logic Base State) ---
    // Chỉ gọi fitView nếu m_isAutoFollow đang bật và người dùng không can thiệp
    if(m_isAutoFollow && m_needsReposition && !m_isPanning)
    {
        fitView(false);

        // Dừng reposition khi các lò xo đã ổn định
        if (std::abs(m_viewCenterSpring.velocity.x) < 0.1f &&
            std::abs(m_viewCenterSpring.velocity.y) < 0.1f)
        {
            m_needsReposition = false;
        }

//        if (std::abs(m_viewCenterSpring.target.x - m_viewCenterSpring.position.x) < 0.1f &&
//            std::abs(m_viewCenterSpring.target.y - m_viewCenterSpring.position.y) < 0.1f)
//        {
//            m_needsReposition = false;
//        }
    }
}

    void StructurePanel::renderContent()
    {
        // Không cần frame hay dt ở đây nữa, vì m_nodeUIMap và m_edgeUIMap đã được update xong xuôi
        if(m_contentBuffer.getSize().x == 0) return;

        m_contentBuffer.clear(sf::Color::Transparent);
        m_contentBuffer.setView(m_view);

        // 1. Vẽ Layer Dây (Nằm dưới)
        for(const auto& pair : m_edgeUIMap)
        {
            pair.second->draw(m_contentBuffer);

//            std::cout << "DRAW EDGE!\n";
        }

        // 2. Vẽ Layer Node (Nằm trên)
        for(const auto& pair : m_nodeUIMap)
        {
            pair.second->draw(m_contentBuffer);
//            std::cout << "DRAW NODE!\n";
        }

        m_contentBuffer.display();
    }

    // StructurePanel.cpp

    sf::FloatRect StructurePanel::calculateBoundingBox()
    {
        if (m_nodeUIMap.empty())
        {
            // Trả về một vùng mặc định nếu không có node nào
            return sf::FloatRect(0.f, 0.f, 1.f, 1.f);
        }

        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();

        for (const auto& pair : m_nodeUIMap)
        {
            sf::Vector2f pos = pair.second->getCurrentPosition();
            float radius = 25.0f; // Bán kính node (nên lấy từ NodeUI nếu có thể)

            minX = std::min(minX, pos.x - radius);
            minY = std::min(minY, pos.y - radius);
            maxX = std::max(maxX, pos.x + radius);
            maxY = std::max(maxY, pos.y + radius);
        }

        return sf::FloatRect(minX, minY, maxX - minX, maxY - minY);
    }

    void StructurePanel::fitView(bool immediate)
    {
        if(m_nodeUIMap.empty())
        {
            // If empty, return to a default state (e.g., center of the buffer)
            sf::Vector2f defaultCenter(1920.f / 2.f, 1080.f / 2.f);
            if(immediate)
            {
                m_zoomSpring.snapTo(1.0f);
                m_viewCenterSpring.snapTo(defaultCenter);
            }
            else
            {
                m_zoomSpring.target = 1.0f;
                m_viewCenterSpring.target = defaultCenter;
            }
            return;
        }

        sf::FloatRect bbox = calculateBoundingBox();
//        float w = m_frame->getSize().x;
//        float h = m_frame->getSize().y;

        float w = m_widthSpring.target;
        float h = m_heightSpring.target;

        if(w <= 0 || h <= 0) return;


//        std::cout << "[PREVIOUS TARGET SIZE]: " << w << " " << h << "\n";
//        std::cout << "[BOUNDING BOX]: " << bbox.left << " " << bbox.top << " " << bbox.width << " " << bbox.height << "\n";

        // Calculate target zoom with padding
        float padding = 80.0f;
        float targetZoomX = (bbox.width + padding * 2.0f) / w;
        float targetZoomY = (bbox.height + padding * 2.0f) / h;
        float targetZoom = std::max(targetZoomX, targetZoomY);

        // Limit zoom range
        targetZoom = std::max(0.6f, std::min(targetZoom, 4.0f));

//        std::cout << "[TARGET ZOOM]: " << targetZoom << "\n";

        sf::Vector2f bboxCenter(bbox.left + bbox.width / 2.0f, bbox.top + bbox.height / 2.0f);

//        std::cout << "[BOUNDING BOX CENTER]: " << bboxCenter.x << " " << bboxCenter.y << "\n";

        // Offset for the Crop system to keep things centered in the 1920x1080 buffer
        sf::Vector2f offset;
        offset.x = (1920.0f - w) / 2.0f;
        offset.y = (1080.0f - h) / 2.0f;

        sf::Vector2f targetCenter = bboxCenter + (offset * targetZoom);

//        std::cout << "[TARGET VIEW CENTER]: " << targetCenter.x << " " << targetCenter.y << "\n";

        if(immediate)
        {
            m_zoomSpring.snapTo(targetZoom);
            m_viewCenterSpring.snapTo(targetCenter);
        }
        else
        {
            m_zoomSpring.target = targetZoom;
            m_viewCenterSpring.target = targetCenter;
        }
    }

    void StructurePanel::zoomAt(float delta, sf::Vector2f mouseWorldPos)
    {
        float zoomFactor = (delta > 0) ? 0.9f : 1.1f;
        sf::Vector2f before = mouseWorldPos;
        m_view.zoom(zoomFactor);
        m_currentZoom *= zoomFactor;
        m_view.setCenter(m_view.getCenter() + (before - m_view.getCenter()) * (1.0f - zoomFactor));
    }

    void StructurePanel::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // 1. Vẽ nền kính mờ (giữ nguyên)
        target.draw(*m_frame, states);

        float w = m_frame->getSize().x;
        float h = m_frame->getSize().y;


        if(w > 0 && h > 0)
        {
            sf::Sprite contentSprite(m_contentBuffer.getTexture());

            // SỬA TẠI ĐÂY: Chỉ lấy phần texture tương ứng với kích thước Panel hiện tại
            // Giống hệt cách bạn làm với Glass Texture trong Squircle.cpp
            contentSprite.setTextureRect(sf::IntRect(0, 0, (int)w, (int)h));

            contentSprite.setPosition(m_frame->getPosition());

            if(m_isMaskLoaded)
            {

                sf::Vector2f halfSize(w * 0.5f, h * 0.5f);
                sf::Vector2f innerBound(halfSize.x - 30.f, halfSize.y - 30.f);

                m_maskShader.setUniform("u_texture", sf::Shader::CurrentTexture);
                m_maskShader.setUniform("u_halfSize", halfSize);
                m_maskShader.setUniform("u_innerBound", innerBound);
                m_maskShader.setUniform("u_radius", 30.f);
//                m_maskShader.setUniform("u_texture", sf::Shader::CurrentTexture);
//                m_maskShader.setUniform("u_panelSize", sf::Vector2f(w, h));
//                m_maskShader.setUniform("u_radius", 30.0f);
//                m_maskShader.setUniform("u_power", 4.0f);
                states.shader = &m_maskShader;
            }

            target.draw(contentSprite, states);
        }

        // 2. Vẽ Empty Message nếu không có node
        if(m_nodeUIMap.empty())
        {
            // Có thể nhân thêm m_alphaSpring.position nếu muốn chữ cũng fade out mượt mà
            target.draw(m_emptyMessage);
        }

        resetBtn->draw(target);
    }

    sf::FloatRect StructurePanel::getGlobalBounds() const
    {
        if (m_frame)
        {
            return m_frame->getGlobalBounds();
        }
        return sf::FloatRect();
    }
}
