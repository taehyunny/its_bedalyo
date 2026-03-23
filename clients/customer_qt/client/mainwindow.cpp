#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))
    , m_loginWidget(new LoginWidget(m_network, this))
    , m_homeWidget(new HomeWidget(m_network, this))
    , m_menuWidget(new menucategori(m_network, this))
    , m_searchWidget(new SearchWidget(m_network, this))
    , m_searchResultWidget(new SearchResultWidget(m_network, this))
    , m_orderHistoryWidget(new OrderHistoryWidget(m_network, this))
    , m_myPageWidget(new MyPageWidget(m_network, this))
    , m_storeDetailWidget(new StoreDetailWidget(m_network, this))
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);
    setFixedSize(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    // ── 화면 등록 ──
    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->addWidget(m_menuWidget);
    ui->stackedWidget->addWidget(m_searchWidget);
    ui->stackedWidget->addWidget(m_searchResultWidget);
    ui->stackedWidget->addWidget(m_orderHistoryWidget);
    ui->stackedWidget->addWidget(m_myPageWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_storeDetailWidget);

    // ── 로그인 ──
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_homeWidget, &HomeWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    // ── 카테고리 화면 ──
    connect(m_homeWidget, &HomeWidget::categorySelected,
            this, &MainWindow::onCategorySelected);
    connect(m_menuWidget, &menucategori::backRequested,
            this, &MainWindow::onBackToHome);

    // ── 검색 화면 ──
    connect(m_homeWidget, &HomeWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchWidget, &SearchWidget::backRequested,
            this, &MainWindow::onBackToHome);
    connect(m_searchWidget, &SearchWidget::searchRequested,
            this, &MainWindow::onSearchExecuted);
    connect(m_searchResultWidget, &SearchResultWidget::backRequested,
            this, &MainWindow::onSearchRequested);

    // ── 검색 내비바 ──
    connect(m_searchWidget, &SearchWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_searchWidget, &SearchWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);
    connect(m_searchWidget, &SearchWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);

    // ── 주문내역 화면 ──
    connect(m_homeWidget, &HomeWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::homeRequested,
            this, &MainWindow::onBackToHome);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);

    // ── 마이페이지 화면 ──
    connect(m_homeWidget, &HomeWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);
    connect(m_myPageWidget, &MyPageWidget::homeRequested,
            this, &MainWindow::onBackToHome);
    connect(m_myPageWidget, &MyPageWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_myPageWidget, &MyPageWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_myPageWidget, &MyPageWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);
    connect(m_myPageWidget, &MyPageWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    // ── 카테고리 데이터 캐싱 ──
    connect(m_network, &NetworkManager::onMainHomeReceived,
            this, &MainWindow::onMainHomeReceived);

    // ── 서버 연결 ──
    m_network->connectToServer(AppConfig::SERVER_IP, AppConfig::SERVER_PORT);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onLoginSuccess()
{
    m_homeWidget->setUserName(UserSession::instance().userName);
    m_homeWidget->setAddress(UserSession::instance().address);
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onLogoutRequested()
{
    UserSession::instance().clear();
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                     QList<TopStoreInfoQt> /*topStores*/)
{
    m_cachedCategories = categories;
}

void MainWindow::onCategorySelected(int categoryId, const QString &categoryName)
{
    m_menuWidget->setCategory(categoryId, categoryName, m_cachedCategories);
    ui->stackedWidget->setCurrentWidget(m_menuWidget);
}

void MainWindow::onSearchRequested()
{
    m_searchWidget->loadSearchData();
    ui->stackedWidget->setCurrentWidget(m_searchWidget);
}

void MainWindow::onSearchExecuted(const QString &keyword)
{
    m_searchResultWidget->search(keyword);
    ui->stackedWidget->setCurrentWidget(m_searchResultWidget);
}

void MainWindow::onOrderListRequested()
{
    m_orderHistoryWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_orderHistoryWidget);
}

void MainWindow::onMypageRequested()
{
    m_myPageWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_myPageWidget);
}

void MainWindow::onBackToHome()
{
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

// ============================================================
// 즐겨찾기 화면 전환
// TODO: 즐겨찾기 Widget 구현 후 화면 전환 추가
// ============================================================
void MainWindow::onFavoriteRequested()
{
    // TODO: m_favoriteWidget->loadData();
    // ui->stackedWidget->setCurrentWidget(m_favoriteWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }

// ============================================================
// 2페이지(가게 목록)에서 가게를 클릭했을 때 실행되는 함수
// ============================================================
void MainWindow::onStoreSelected(int storeId)
{
    qDebug() << "[MainWindow] 3페이지로 이동! 선택된 가게 ID:" << storeId;
    
    // 🚀 이제 에러 안 납니다! 가게 ID를 3페이지로 던져줍니다.
    m_storeDetailWidget->loadStoreData(storeId);
    
    // 화면을 3페이지로 전환!
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}
