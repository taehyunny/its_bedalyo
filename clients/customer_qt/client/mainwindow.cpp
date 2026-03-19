#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>

// ============================================================
// 생성자
// 1. NetworkManager 생성 (서버 연결 포함)
// 2. 각 Widget 생성 및 QStackedWidget에 등록
// 3. 시그널-슬롯 연결
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))      // 통신 객체 생성 (MainWindow가 소유)
    , m_loginWidget(new LoginWidget(m_network, this)) // NetworkManager 주입
{
    ui->setupUi(this);

    // ── QStackedWidget에 화면 등록 ──
    // 향후 화면 추가 시 아래에 addWidget 추가
    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget); // 첫 화면 = 로그인

    // ── 시그널-슬롯 연결 ──
    // 서버 연결 성공 → 상태 라벨 업데이트
    connect(m_network,     &NetworkManager::onConnected,
            this,          &MainWindow::onConnected);

    // 로그인 성공 → 다음 화면으로 전환
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this,          &MainWindow::onLoginSuccess);

    // ── 서버 연결 시도 ──
    // TODO: IP/Port를 config.h 상수로 분리 권장
    // #include "config.h" → m_network->connectToServer(SERVER_IP, SERVER_PORT);
    m_network->connectToServer("10.10.10.123", 8080);
}

MainWindow::~MainWindow()
{
    delete ui;
    // m_network, m_loginWidget은 QObject 부모-자식 관계로 자동 소멸
}

// ============================================================
// 서버 연결 성공 슬롯
// ============================================================
void MainWindow::onConnected()
{
    ui->statusLabel->setText("✅ 서버 연결됨");
}

// ============================================================
// 로그인 성공 슬롯
// 현재: 상태 라벨 업데이트
// 향후: HomeWidget으로 화면 전환
// ============================================================
void MainWindow::onLoginSuccess(QString userName)
{
    // TODO: HomeWidget 구현 후 아래 주석 해제
    // ui->stackedWidget->setCurrentWidget(m_homeWidget);

    ui->statusLabel->setText("환영합니다, " + userName + "님! 🎉");
}
