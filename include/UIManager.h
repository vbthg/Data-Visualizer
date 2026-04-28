namespace GUI
{
    class UIManager
    {
    public:
        static UIManager& getInstance();

        // Khởi tạo các component dùng chung (Dock, History, Notch)
        void initGlobalUI(const sf::Font& iconFont, const sf::Font& textFont);

        // Truy xuất an toàn
        FloatingDock* getDock();
        HistoryBoard* getHistoryBoard();
        DynamicIsland* getNotch();

        // Xử lý sự kiện tập trung cho UI
        void handleGlobalEvents(const sf::Event& event, sf::RenderWindow& window);
        void update(float dt, sf::RenderWindow& window);

    private:
        UIManager() = default;
        ~UIManager(); // Thực hiện delete sạch sẽ các con trỏ ở đây

        FloatingDock* m_dock = nullptr;
        HistoryBoard* m_history = nullptr;
        DynamicIsland* m_notch = nullptr;
    };
}
