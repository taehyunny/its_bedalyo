#include "mainwindow.h"
#include "ui_mainwindow.h"

// ============================================================
// 생성자
// 1. NetworkManager 생성 (서버 연결 포함)
// 2. 각 Widget 생성 및 QStackedWidget에 등록
// 3. 시그널-슬롯 연결
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))
    , m_loginWidget(new LoginWidget(m_network, this))
    , m_homeWidget(new HomeWidget(m_network, this))
{
    ui->setupUi(this);

    // ── 창 크기 고정 (390x844 모바일 비율) ──
    setFixedSize(390, 844);

    // ── QStackedWidget에 화면 등록 ──
    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget); // 첫 화면 = 로그인

    // ── 시그널-슬롯 연결 ──
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);

    connect(m_homeWidget, &HomeWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    // ── 서버 연결 시도 ──
    m_network->connectToServer("10.10.10.123", 8080);
}

MainWindow::~MainWindow()
{
    delete ui;
    // m_network, m_loginWidget, m_homeWidget은 QObject 부모-자식 관계로 자동 소멸
}

// ============================================================
// 로그인 성공 슬롯 → 홈 화면으로 전환
// ============================================================
void MainWindow::onLoginSuccess(QString userName)
{
    m_homeWidget->setUserName(userName); // 홈에 유저 이름 전달
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

// ============================================================
// 로그아웃 슬롯 → 로그인 화면으로 복귀
// ============================================================
void MainWindow::onLogoutRequested()
{
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::showLogin()
{
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::showHome()
{
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}
