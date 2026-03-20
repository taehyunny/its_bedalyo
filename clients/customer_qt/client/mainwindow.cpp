#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))
    , m_loginWidget(new LoginWidget(m_network, this))
    , m_homeWidget(new HomeWidget(m_network, this))
{
    // ── 타이틀바 제거 (setupUi 이전에 호출해야 적용됨) ──
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->setupUi(this);
    setFixedSize(390, 844);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);

    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_homeWidget, &HomeWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    m_network->connectToServer("10.10.10.123", 8010);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onLoginSuccess(QString userName, QString address)
{
    m_homeWidget->setUserName(userName);
    m_homeWidget->setAddress(address);
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onLogoutRequested()
{
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
