#include "PseudoCodeBox.h"
#include "ViewHandler.h"
#include <iostream>
#include <algorithm>

namespace GUI
{
    PseudoCodeBox::PseudoCodeBox(const sf::Font& font, float windowWidth, float windowHeight)
        : m_font(font), m_windowWidth(windowWidth), m_windowHeight(windowHeight),
          m_isExpanded(false), m_currentLine(0) // Mặc định thu nhỏ khi mới vào
    {
        // 1. Setup Màu sắc
        m_background.setFillColor(sf::Color(30, 30, 30, 240));
        m_background.setOutlineThickness(1.0f);
        m_background.setOutlineColor(sf::Color(255, 255, 255, 30));

        // 4. Setup Thanh Highlighter (Đánh dấu dòng)
        m_highlighter.setFillColor(sf::Color(0, 122, 255, 60)); // Xanh Apple mờ (rất sang trọng trên nền tối)
        m_highlighter.setRadius(6.0f); // Bo góc nhẹ cho ôm chữ

        // Setup Icon khi thu nhỏ
        m_collapsedIconText.setFont(m_font);
        m_collapsedIconText.setString("</>");
        m_collapsedIconText.setCharacterSize(16);
        m_collapsedIconText.setFillColor(sf::Color::White);

        // 2. Setup Vật lý Morphing (Cứng cáp, nảy nhẹ)
        float stiff = 200.0f;
        float damp = 22.0f;

        m_wSpring.stiffness = stiff; m_wSpring.damping = damp;
        m_hSpring.stiffness = stiff; m_hSpring.damping = damp;
        m_xSpring.stiffness = stiff; m_xSpring.damping = damp;
        m_ySpring.stiffness = stiff; m_ySpring.damping = damp;

        // Lò xo trượt của thanh đánh dấu (Nhanh và dứt khoát)
        m_highlightYSpring.stiffness = 250.0f;
        m_highlightYSpring.damping = 20.0f;

        // Alpha nảy nhanh hơn để giấu chữ sớm trước khi mép cắt chạm vào chữ
        m_alphaSpring.stiffness = 300.0f;
        m_alphaSpring.damping = 25.0f;

        // 3. Khởi tạo vị trí tàng hình hoặc thu nhỏ mặc định
        float startW = 80.0f;
        float startH = 40.0f;
        float startX = m_windowWidth - startW - 20.0f;
        float startY = m_windowHeight / 2.0f - startH / 2.0f; // Lơ lửng giữa lề phải

        m_wSpring.snapTo(startW);
        m_hSpring.snapTo(startH);
        m_xSpring.snapTo(startX);
        m_ySpring.snapTo(startY);
        m_alphaSpring.snapTo(0.0f);
    }

    PseudoCodeBox::~PseudoCodeBox()
    {
    }

    void PseudoCodeBox::toggleState()
    {
        m_isExpanded = !m_isExpanded;

        if (m_isExpanded)
        {
            // Trạng thái BẢNG MỞ
            float targetW = 400.0f;
            float targetH = 500.0f;
            m_wSpring.target = targetW;
            m_hSpring.target = targetH;
            m_xSpring.target = m_windowWidth - targetW - 20.0f;
            m_ySpring.target = (m_windowHeight - targetH) / 2.0f;
            m_alphaSpring.target = 255.0f;
        }
        else
        {
            // Trạng thái VIÊN THUỐC
            float targetW = 20.0f;
            float targetH = 60.0f;
            m_wSpring.target = targetW;
            m_hSpring.target = targetH;
            m_xSpring.target = m_windowWidth - targetW - 20.0f;
            m_ySpring.target = (m_windowHeight - targetH) / 2.0f;
            m_alphaSpring.target = 0.0f; // Lập tức mờ chữ
        }
    }

    void PseudoCodeBox::onResize(float windowWidth, float windowHeight)
    {
        m_windowWidth = windowWidth;
        m_windowHeight = windowHeight;

        // Gọi lại toggleState (giữ nguyên cờ hiện tại) để cập nhật lại Target theo kích thước màn hình mới
        m_isExpanded = !m_isExpanded;
        toggleState();
    }

    void PseudoCodeBox::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);

            // Nếu click trúng viên thuốc hoặc bảng thì bật/tắt
//            if (m_background.getGlobalBounds().contains(mousePos))
            if (Utils::ViewHandler::isMouseInFrame(mousePos, window, m_background.getGlobalBounds()))
            {
                toggleState();
            }
        }
    }

    void PseudoCodeBox::update(float dt)
    {
        // Cập nhật hệ lò xo
        m_wSpring.update(dt);
        m_hSpring.update(dt);
        m_xSpring.update(dt);
        m_ySpring.update(dt);
        m_alphaSpring.update(dt);
        // Cập nhật lò xo highlight
        m_highlightYSpring.update(dt);
        // Cập nhật lò xo nảy của từng biến
        for (auto& var : m_variables)
        {
            var.bounceSpring.update(dt);
        }


        // Áp dụng Geometry
        m_background.setSize({m_wSpring.position, m_hSpring.position});
        m_background.setPosition(m_xSpring.position, m_ySpring.position);

        // Công thức tính Radius thông minh: Không bao giờ vượt quá 20px,
        // nhưng khi thu nhỏ sẽ biến thành bán nguyệt (Pill-shape)
        float currentRadius = std::min(20.0f, m_hSpring.position / 2.0f);
        m_background.setRadius(currentRadius);

        // Căn giữa icon </>. Tọa độ này liên tục chạy theo lò xo
        sf::FloatRect iconBounds = m_collapsedIconText.getLocalBounds();
        m_collapsedIconText.setOrigin(iconBounds.left + iconBounds.width / 2.0f,
                                      iconBounds.top + iconBounds.height / 2.0f);
        m_collapsedIconText.setPosition(m_xSpring.position + m_wSpring.position / 2.0f,
                                        m_ySpring.position + m_hSpring.position / 2.0f);
    }

    void PseudoCodeBox::draw(sf::RenderWindow& window)
    {
        // 1. Vẽ nền (Viên thuốc hoặc Bảng)
        window.draw(m_background);

        // 2. Nếu đang thu nhỏ (Alpha của dàn chữ bên trong < 10) thì vẽ Icon </>
        if (m_alphaSpring.position < 250.0f && !m_isExpanded)
        {
            // Làm icon hiện dần lên khi chữ mờ đi
            sf::Color iconColor = m_collapsedIconText.getFillColor();
            iconColor.a = static_cast<sf::Uint8>(255.0f - std::max(0.0f, std::min(m_alphaSpring.position, 255.0f)));
            m_collapsedIconText.setFillColor(iconColor);
            window.draw(m_collapsedIconText);
        }

        // 3. Nếu bảng đang mở (hoặc đang mở dở), vẽ nội dung với glScissor
        if (m_alphaSpring.position > 5.0f)
        {
            glEnable(GL_SCISSOR_TEST);

            // Cắt đúng bằng kích thước hình nền hiện tại (Thụt vào 5px để không lẹm viền)
            float clipX = m_xSpring.position + 5.0f;
            float clipY = m_ySpring.position + 5.0f;
            float clipW = std::max(0.0f, m_wSpring.position - 10.0f);
            float clipH = std::max(0.0f, m_hSpring.position - 10.0f);

            float glY = m_windowHeight - (clipY + clipH);

            glScissor(static_cast<GLint>(clipX), static_cast<GLint>(glY),
                      static_cast<GLsizei>(clipW), static_cast<GLsizei>(clipH));

            // =====================================
            // TODO: Vẽ m_titleText, m_parsedCode, m_variables ở đây
            // (Nhớ set FillColor có alpha bằng biến m_alphaSpring.position trước khi vẽ)
            // =====================================

            // (Nằm bên trong block glEnable(GL_SCISSOR_TEST))

            // --- 1. Vẽ Tiêu đề (Title) ---
            m_titleText.setPosition(m_xSpring.position + 20.0f, m_ySpring.position + 15.0f);

            // Ép độ mờ (Alpha) theo hiệu ứng Morphing
            sf::Color tColor = m_titleText.getFillColor();
            tColor.a = static_cast<sf::Uint8>(std::max(0.0f, std::min(m_alphaSpring.position, 255.0f)));
            m_titleText.setFillColor(tColor);

            window.draw(m_titleText);



            // --- 1.5. VẼ THANH HIGHLIGHTER TRƯỢT ---
            // Chiều rộng bằng khung trừ đi 2 bên lề (20px * 2)
            float hlWidth = m_wSpring.position - 40.0f;
            // Chiều cao bao trọn dòng chữ (lineSpacing + padding)
            float hlHeight = 28.0f + 8.0f;

            m_highlighter.setSize({std::max(0.0f, hlWidth), hlHeight});

            // Tọa độ X giữ cố định (cách lề trái 20), Tọa độ Y thì đuổi theo lò xo
            m_highlighter.setPosition(m_xSpring.position + 20.0f, m_ySpring.position + m_highlightYSpring.position);

            // Đồng bộ Alpha để nó cũng mờ đi khi bảng thu nhỏ thành viên thuốc
            sf::Color hlColor = m_highlighter.getFillColor();
            // Map từ dải alphaSpring (0-255) sang độ mờ gốc của thanh (0-60)
            hlColor.a = static_cast<sf::Uint8>(std::max(0.0f, std::min(60.0f, m_alphaSpring.position * (60.0f / 255.0f))));
            m_highlighter.setFillColor(hlColor);

            window.draw(m_highlighter);



            // --- 2. Vẽ Các dòng Code đã được Parse ---
            float startY = m_ySpring.position + 60.0f; // Bắt đầu vẽ cách viền trên 60px
            float lineSpacing = 28.0f;                 // Khoảng cách mỗi dòng code

            for (size_t lineIdx = 0; lineIdx < m_parsedCode.size(); ++lineIdx)
            {
                float currentX = m_xSpring.position + 20.0f; // Lề trái
                float currentY = startY + (lineIdx * lineSpacing);

                for (auto& token : m_parsedCode[lineIdx])
                {
                    token.text.setPosition(currentX, currentY);

                    // Đồng bộ Alpha với Morphing
                    sf::Color c = token.text.getFillColor();
                    c.a = static_cast<sf::Uint8>(std::max(0.0f, std::min(m_alphaSpring.position, 255.0f)));
                    token.text.setFillColor(c);

                    window.draw(token.text);

                    // Dịch X sang phải một đoạn bằng đúng chiều rộng của chữ vừa vẽ
                    currentX += token.text.getLocalBounds().width;
                }
            }

            glDisable(GL_SCISSOR_TEST);

            // GỌI HÀM VẼ BIẾN Ở ĐÂY:
            drawVariables(window);
        }
    }

    void PseudoCodeBox::parseLine(const std::string& line, int lineIndex)
    {
        std::vector<Token> tokens;
        std::string currentWord = "";

        // Hàm Lambda phụ trợ: Nhận 1 từ, tô màu và đẩy vào mảng tokens
        auto addToken = [&](std::string word)
        {
            if (word.empty()) return;

            Token t;
            t.text.setFont(m_font);
            t.text.setString(word);
            t.text.setCharacterSize(16); // Cỡ chữ chuẩn IDE

            // TÔ MÀU CÚ PHÁP (Syntax Highlighting)
            sf::Color textColor = sf::Color(220, 220, 220); // Mặc định Trắng xám (Biến thông thường)

            if (word == "if" || word == "else" || word == "while" || word == "for" || word == "return" || word == "break")
                textColor = sf::Color(255, 105, 180); // Hồng (Keywords điều khiển)

            else if (word == "int" || word == "bool" || word == "void" || word == "Node*" || word == "Node" || word == "Vertex")
                textColor = sf::Color(86, 156, 214); // Xanh dương (Kiểu dữ liệu)

            else if (word == "nullptr" || word == "null" || word == "true" || word == "false" || (word[0] >= '0' && word[0] <= '9'))
                textColor = sf::Color(206, 145, 120); // Cam (Hằng số, giá trị rỗng)

            else if (word == "//")
                textColor = sf::Color(106, 153, 85); // Xanh lá mờ (Comment)

            t.text.setFillColor(textColor);
            tokens.push_back(t);
        };

        // Duyệt từng ký tự trong dòng code để cắt
        bool isComment = false;
        for (size_t i = 0; i < line.size(); ++i)
        {
            char c = line[i];

            // Nếu là khoảng trắng hoặc ký tự đặc biệt -> Cắt từ hiện tại ra
            if (c == ' ' || c == '(' || c == ')' || c == ';' || c == '=' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == ',' || c == '{' || c == '}')
            {
                addToken(currentWord);
                currentWord = ""; // Xóa bộ đệm

                // Giữ lại khoảng trắng hoặc dấu câu thành 1 token riêng để vẽ
                addToken(std::string(1, c));
            }
            else
            {
                currentWord += c; // Tích lũy chữ cái
            }
        }

        // Đẩy từ cuối cùng vào (nếu còn sót lại)
        addToken(currentWord);

        // Đẩy cả dòng (đã tách) vào mảng 2 chiều
        m_parsedCode.push_back(tokens);
    }

    void PseudoCodeBox::loadCode(const std::string& title, const std::vector<std::string>& codeLines)
    {
        // Cập nhật tiêu đề cửa sổ
        m_titleText.setFont(m_font);
        m_titleText.setString(title);
        m_titleText.setCharacterSize(16);
        m_titleText.setFillColor(sf::Color::White);

        // Xóa code cũ
        m_parsedCode.clear();

        // Phân tích và nạp code mới
        for (size_t i = 0; i < codeLines.size(); ++i)
        {
            parseLine(codeLines[i], i);
        }

        // Reset các biến liên quan
        m_currentLine = 0;
        m_highlightYSpring.snapTo(0.0f); // Reset lò xo đánh dấu về dòng đầu tiên
    }

    void PseudoCodeBox::updateStep(int currentLineIndex, const std::vector<std::pair<std::string, std::string>>& variables)
    {
        m_currentLine = currentLineIndex;

        // Tính toán tọa độ Y đích (Target Y) cho thanh đánh dấu
        // startY (điểm bắt đầu vẽ code) = 60.0px, Khoảng cách mỗi dòng (lineSpacing) = 28.0px
        // Ta trừ đi 4.0px để padding thanh màu nhô lên trên chữ một chút cho đẹp
        float targetY = 60.0f + (m_currentLine * 28.0f) - 4.0f;

        // Đặt mục tiêu cho lò xo chạy tới đó
        m_highlightYSpring.target = targetY;

        // (Phần lưu dữ liệu Variables mình sẽ xử lý ở bước tiếp theo)

        // 2. Cập nhật danh sách Biến (Variables)
        std::vector<VariableUI> updatedVariables;

        for (const auto& pair : variables)
        {
            std::string varName = pair.first;
            std::string varValue = pair.second;

            // Tìm xem biến này đã có trên bảng chưa
            auto it = std::find_if(m_variables.begin(), m_variables.end(), [&](const VariableUI& v)
            {
                return v.nameText.getString() == varName;
            });

            if (it != m_variables.end())
            {
                // Biến ĐÃ TỒN TẠI -> Kiểm tra xem giá trị có đổi không
                if (it->valueText.getString() != varValue)
                {
                    it->valueText.setString(varValue);
                    // Bí quyết hiệu ứng: Nén lò xo về 0.3 để nó tự bung dội lại lên 1.0
                    it->bounceSpring.position = 0.3f;
                }
                updatedVariables.push_back(*it);
            }
            else
            {
                // Biến MỚI HOÀN TOÀN -> Khởi tạo UI cho nó
                VariableUI newVar;

                newVar.nameText.setFont(m_font);
                newVar.nameText.setString(varName);
                newVar.nameText.setCharacterSize(14);
                newVar.nameText.setFillColor(sf::Color(170, 170, 170)); // Xám sáng

                newVar.valueText.setFont(m_font);
                newVar.valueText.setString(varValue);
                newVar.valueText.setCharacterSize(15);
                // Nếu là null/nullptr thì tô Cam, ngược lại tô Xanh lơ (Cyan)
                if (varValue == "null" || varValue == "nullptr")
                    newVar.valueText.setFillColor(sf::Color(206, 145, 120));
                else
                    newVar.valueText.setFillColor(sf::Color(78, 201, 176));

                // Cài đặt lò xo giật nảy
                newVar.bounceSpring.stiffness = 300.0f;
                newVar.bounceSpring.damping = 15.0f;
                newVar.bounceSpring.target = 1.0f;
                newVar.bounceSpring.position = 0.3f; // Nảy lên lúc mới xuất hiện

                updatedVariables.push_back(newVar);
            }
        }

        // Thay thế mảng cũ bằng mảng mới (những biến không được truyền vào sẽ tự động bị xóa)
        m_variables = std::move(updatedVariables);
    }

    void PseudoCodeBox::drawVariables(sf::RenderWindow& window)
    {
        // Nếu bảng đang thu nhỏ thành viên thuốc thì không vẽ để tránh rác màn hình
        if (m_alphaSpring.position < 5.0f) return;

        // Lấy Alpha chung của cả bảng
        sf::Uint8 globalAlpha = static_cast<sf::Uint8>(std::max(0.0f, std::min(m_alphaSpring.position, 255.0f)));

        // 1. Vẽ đường kẻ ngăn cách (Separator Line)
        // Cố định ở 150px tính từ đáy bảng hắt lên
        float separatorY = m_ySpring.position + m_hSpring.position - 150.0f;

        sf::RectangleShape separator({std::max(0.0f, m_wSpring.position - 40.0f), 1.0f});
        separator.setPosition(m_xSpring.position + 20.0f, separatorY);
        separator.setFillColor(sf::Color(255, 255, 255, globalAlpha / 4)); // Trắng mờ 25%
        window.draw(separator);

        // 2. Vẽ Tiêu đề phụ
        sf::Text title;
        title.setFont(m_font);
        title.setString("LOCAL VARIABLES");
        title.setCharacterSize(12);
        title.setFillColor(sf::Color(120, 120, 120, globalAlpha));
        title.setPosition(m_xSpring.position + 20.0f, separatorY + 10.0f);
        window.draw(title);

        // 3. Vẽ danh sách các biến
        float varStartY = separatorY + 35.0f; // Vị trí dòng biến đầu tiên
        float lineSpacing = 24.0f;

        for (size_t i = 0; i < m_variables.size(); ++i)
        {
            auto& var = m_variables[i];
            float currentY = varStartY + (i * lineSpacing);

            // --- Vẽ Tên biến (Cột trái) ---
            var.nameText.setPosition(m_xSpring.position + 20.0f, currentY);
            sf::Color nameCol = var.nameText.getFillColor();
            nameCol.a = globalAlpha;
            var.nameText.setFillColor(nameCol);
            window.draw(var.nameText);

            // --- Vẽ Giá trị biến (Cột phải) ---
            // Neo ở vị trí X = 130 để tạo thành một cột thẳng tắp nhìn rất pro
            float valueX = m_xSpring.position + 130.0f;

            sf::Color valCol = var.valueText.getFillColor();
            valCol.a = globalAlpha;
            var.valueText.setFillColor(valCol);

            // Áp dụng hiệu ứng giật nảy (Scale Bounce)
            float scale = var.bounceSpring.position;
            var.valueText.setScale(scale, scale);

            // Dịch tâm (Origin) về giữa chiều cao của chữ để lúc nảy lên nó phình ra từ tâm, không bị rớt xuống dưới
            sf::FloatRect bounds = var.valueText.getLocalBounds();
            var.valueText.setOrigin(0.0f, bounds.height / 2.0f);
            var.valueText.setPosition(valueX, currentY + bounds.height / 2.0f);

            window.draw(var.valueText);
        }
    }
}

