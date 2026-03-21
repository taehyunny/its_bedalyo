#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"

// ============================================================
// 생성자
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))
    , m_loginWidget(new LoginWidget(m_network, this))
    , m_homeWidget(new HomeWidget(m_network, this))
{
    // 타이틀바 제거 (setupUi 이전에 호출해야 적용됨)
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->setupUi(this);
    setFixedSize(390, 844);

    // QMainWindow 기본 여백 제거
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    // 화면 등록
    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);

    // 시그널 연결
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_homeWidget, &HomeWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    // 서버 연결
    m_network->connectToServer("10.10.10.123", 8012);
}

MainWindow::~MainWindow() { delete ui; }

// ============================================================
// 로그인/회원가입 성공 → UserSession에서 정보 꺼내 홈 화면에 적용
// ============================================================
void MainWindow::onLoginSuccess()
{
    // UserSession에 저장된 값을 HomeWidget에 전달
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

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
