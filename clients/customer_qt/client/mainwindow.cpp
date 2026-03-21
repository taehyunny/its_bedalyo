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
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);
    setFixedSize(390, 844);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    // ── 화면 등록 ──
    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->addWidget(m_menuWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);

    // ── 시그널 연결 ──
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_homeWidget, &HomeWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);
    connect(m_homeWidget, &HomeWidget::categorySelected,
            this, &MainWindow::onCategorySelected);
    connect(m_menuWidget, &menucategori::backRequested,
            this, &MainWindow::onBackToHome);

    // ── 카테고리 데이터 캐싱 ──
    // HomeWidget이 받은 데이터를 MainWindow도 같이 받아서 캐싱
    // menucategori는 여기서 직접 연결 안 하고 setCategory() 호출 시 전달
    connect(m_network, &NetworkManager::onMainHomeReceived,
            this, &MainWindow::onMainHomeReceived);

    // ── 서버 연결 ──
    m_network->connectToServer("10.10.10.123", 8010);
}

MainWindow::~MainWindow() { delete ui; }

// ============================================================
// 로그인/회원가입 성공 → 홈 화면으로 전환
// ============================================================
void MainWindow::onLoginSuccess()
{
    m_homeWidget->setUserName(UserSession::instance().userName);
    m_homeWidget->setAddress(UserSession::instance().address);
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

// ============================================================
// 로그아웃 → 세션 초기화 후 로그인 화면으로 복귀
// ============================================================
void MainWindow::onLogoutRequested()
{
    UserSession::instance().clear();
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

// ============================================================
// 카테고리 데이터 캐싱
// HomeWidget이 RES_CATEGORY 받을 때 MainWindow도 같이 받아서 저장
// ============================================================
void MainWindow::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                     QList<TopStoreInfoQt> /*topStores*/)
{
    m_cachedCategories = categories;
}

// ============================================================
// 홈에서 카테고리 클릭 → menucategori 화면으로 전환
// 캐싱된 카테고리 목록을 넘겨주고 가게 목록은 서버에 새로 요청
// ============================================================
void MainWindow::onCategorySelected(int categoryId, const QString &categoryName)
{
    m_menuWidget->setCategory(categoryId, categoryName, m_cachedCategories);
    ui->stackedWidget->setCurrentWidget(m_menuWidget);
}

// ============================================================
// menucategori 뒤로가기 → 홈으로 복귀
// ============================================================
void MainWindow::onBackToHome()
{
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
