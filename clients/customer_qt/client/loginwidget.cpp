#include "LoginWidget.h"
#include "ui_loginwidget.h"
#include "AccountDTO.h"
#include <QMessageBox>

// ── 탭 스타일 상수 ──
static const QString TAB_ACTIVE =
    "color: #1565c0;"
    "border-bottom: 2.5px solid #1565c0;"
    "background: transparent;"
    "font-size: 16px;"
    "font-weight: bold;"
    "padding: 12px 0px;"
    "border-top: none; border-left: none; border-right: none;";

static const QString TAB_INACTIVE =
    "color: #bbbbbb;"
    "border-bottom: 2.5px solid transparent;"
    "background: transparent;"
    "font-size: 16px;"
    "font-weight: bold;"
    "padding: 12px 0px;"
    "border-top: none; border-left: none; border-right: none;";

// ============================================================
// 생성자
// ============================================================
LoginWidget::LoginWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 탭 전환: 로그인 ──
    connect(ui->tabLogin, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(0);
        ui->tabLogin->setStyleSheet(TAB_ACTIVE);
        ui->tabSignup->setStyleSheet(TAB_INACTIVE);
    });

    // ── 탭 전환: 회원가입 ──
    connect(ui->tabSignup, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(1);
        ui->tabSignup->setStyleSheet(TAB_ACTIVE);
        ui->tabLogin->setStyleSheet(TAB_INACTIVE);
    });

    // ── 서버 인증 응답 수신 ──
    connect(m_network, &NetworkManager::onAuthResponse,
            this, &LoginWidget::onAuthResponse);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

// ============================================================
// 로그인 버튼 클릭
// ============================================================
void LoginWidget::on_loginButton_clicked()
{
    if (ui->idEdit->text().isEmpty() || ui->pwEdit->text().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "아이디와 비밀번호를 입력해주세요.");
        return;
    }

    LoginReqDTO dto;
    dto.userId   = ui->idEdit->text().toStdString();
    dto.password = ui->pwEdit->text().toStdString();
    m_network->sendLogin(dto);
}

// ============================================================
// 회원가입 버튼 클릭
// ============================================================
void LoginWidget::on_signupButton_clicked()
{
    if (ui->signupIdEdit->text().isEmpty() || ui->signupPwEdit->text().isEmpty()
        || ui->nameEdit->text().isEmpty()   || ui->phoneEdit->text().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "모든 항목을 입력해주세요.");
        return;
    }

    SignupReqDTO dto;
    dto.userId      = ui->signupIdEdit->text().toStdString();
    dto.password    = ui->signupPwEdit->text().toStdString();
    dto.userName    = ui->nameEdit->text().toStdString();
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.role        = 0; // 0: 고객
    m_network->sendSignup(dto);
}

// ============================================================
// 서버 인증 응답 수신 슬롯
// ============================================================
void LoginWidget::onAuthResponse(int status, QString message, QString userName)
{
    if (status == 200 && !userName.isEmpty()) {
        emit loginSuccess(userName);
    } else {
        QString title = (status == 200) ? "성공" : "실패";
        QMessageBox::information(this, title, message);
    }
}
