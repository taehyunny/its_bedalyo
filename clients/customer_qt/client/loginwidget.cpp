#include "LoginWidget.h"   // 대소문자 정확히 일치
#include "ui_loginwidget.h"
#include "AccountDTO.h"
#include <QMessageBox>
#include <QLabel>

static const QString TAB_ACTIVE =
    "color:#1565c0; border-bottom:2.5px solid #1565c0; background:transparent;"
    "font-size:16px; font-weight:bold; padding:10px 0px;"
    "border-top:none; border-left:none; border-right:none;";

static const QString TAB_INACTIVE =
    "color:#bbbbbb; border-bottom:2.5px solid transparent; background:transparent;"
    "font-size:16px; font-weight:bold; padding:10px 0px;"
    "border-top:none; border-left:none; border-right:none;";

LoginWidget::LoginWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // 탭 전환
    connect(ui->tabLogin, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(0);
        ui->tabLogin->setStyleSheet(TAB_ACTIVE);
        ui->tabSignup->setStyleSheet(TAB_INACTIVE);
    });
    connect(ui->tabSignup, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(1);
        ui->tabSignup->setStyleSheet(TAB_ACTIVE);
        ui->tabLogin->setStyleSheet(TAB_INACTIVE);
    });

    // 비밀번호 실시간 확인
    connect(ui->signupPwEdit,        &QLineEdit::textChanged,
            this, &LoginWidget::onPwConfirmChanged);
    connect(ui->signupPwConfirmEdit, &QLineEdit::textChanged,
            this, &LoginWidget::onPwConfirmChanged);

    // 아이디 변경 시 플래그 초기화
    connect(ui->signupIdEdit, &QLineEdit::textChanged, this, [this]() {
        m_idChecked = false;
        setMsgNeutral(ui->label_idMsg, "");
        updateSignupButtonState();
    });

    // 전화번호 변경 시 플래그 초기화
    connect(ui->phoneEdit, &QLineEdit::textChanged, this, [this]() {
        m_phoneChecked = false;
        setMsgNeutral(ui->label_phoneMsg, "");
        updateSignupButtonState();
    });

    // ── 초기 상태: 로그인 탭으로 설정 ──
    ui->formStack->setCurrentIndex(0);

    // 나머지 입력 필드 변경 시 버튼 상태 재검사
    connect(ui->signupPwEdit,   &QLineEdit::textChanged, this, [this](){ updateSignupButtonState(); });
    connect(ui->nameEdit,       &QLineEdit::textChanged, this, [this](){ updateSignupButtonState(); });
    connect(ui->addressEdit,    &QLineEdit::textChanged, this, [this](){ updateSignupButtonState(); });

    // 서버 응답 연결
    connect(m_network, &NetworkManager::onAuthResponse,
            this, &LoginWidget::onAuthResponse);
    connect(m_network, &NetworkManager::onIdCheckResponse,
            this, &LoginWidget::onIdCheckResponse);
    connect(m_network, &NetworkManager::onPhoneCheckResponse,
            this, &LoginWidget::onPhoneCheckResponse);
}

LoginWidget::~LoginWidget() { delete ui; }

// ── 로그인 ──
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

// ── 아이디 중복확인 ──
void LoginWidget::on_btnCheckId_clicked()
{
    if (ui->signupIdEdit->text().isEmpty()) {
        setMsgError(ui->label_idMsg, "아이디를 먼저 입력해주세요.");
        return;
    }
    AuthCheckReqDTO dto;
    dto.userId = ui->signupIdEdit->text().toStdString();
    m_network->sendIdCheck(dto);

    setMsgNeutral(ui->label_idMsg, "확인 중...");
    ui->btnCheckId->setEnabled(false);
}

// ── 전화번호 중복확인 ──
void LoginWidget::on_btnCheckPhone_clicked()
{
    if (ui->phoneEdit->text().isEmpty()) {
        setMsgError(ui->label_phoneMsg, "전화번호를 먼저 입력해주세요.");
        return;
    }
    PhoneCheckReqDTO dto;
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.role        = 0; // 고객
    m_network->sendPhoneCheck(dto);

    setMsgNeutral(ui->label_phoneMsg, "확인 중...");
    ui->btnCheckPhone->setEnabled(false);
}

// ── 비밀번호 실시간 확인 ──
void LoginWidget::onPwConfirmChanged()
{
    QString pw        = ui->signupPwEdit->text();
    QString pwConfirm = ui->signupPwConfirmEdit->text();

    if (pwConfirm.isEmpty()) {
        setMsgNeutral(ui->label_pwMsg, "");
        m_pwMatched = false;
    } else if (pw == pwConfirm) {
        setMsgSuccess(ui->label_pwMsg, "✅ 비밀번호가 일치합니다.");
        m_pwMatched = true;
    } else {
        setMsgError(ui->label_pwMsg, "❌ 비밀번호가 일치하지 않습니다.");
        m_pwMatched = false;
    }
    updateSignupButtonState();
}

// ── 회원가입 버튼 활성화 조건 ──
void LoginWidget::updateSignupButtonState()
{
    bool allFilled = !ui->signupIdEdit->text().isEmpty()
                  && !ui->signupPwEdit->text().isEmpty()
                  && !ui->signupPwConfirmEdit->text().isEmpty()
                  && !ui->nameEdit->text().isEmpty()
                  && !ui->phoneEdit->text().isEmpty()
                  && !ui->addressEdit->text().isEmpty();

    ui->signupButton->setEnabled(allFilled && m_idChecked && m_phoneChecked && m_pwMatched);
}

// ── 회원가입 ──
void LoginWidget::on_signupButton_clicked()
{
    if (!m_idChecked || !m_phoneChecked || !m_pwMatched) return;

    SignupReqDTO dto;
    dto.userId      = ui->signupIdEdit->text().toStdString();
    dto.password    = ui->signupPwEdit->text().toStdString();
    dto.userName    = ui->nameEdit->text().toStdString();
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.address     = ui->addressEdit->text().toStdString();
    dto.role        = 0;
    m_network->sendSignup(dto);
}

// ── 서버 응답 슬롯 ──
void LoginWidget::onAuthResponse(int status, QString message, QString userName)
{
    if (status == 200 && !userName.isEmpty()) {
        // 로그인 성공 → 홈 화면으로 (주소는 로그인 시 서버에서 받아오는 게 이상적이나
        // 현재는 회원가입 시 입력한 주소 사용)
        emit loginSuccess(userName, "");
    } else {
        QString title = (status == 200) ? "성공" : "실패";
        QMessageBox::information(this, title, message);
    }
}

void LoginWidget::onIdCheckResponse(int status, QString message, bool isAvailable)
{
    ui->btnCheckId->setEnabled(true);
    if (isAvailable) {
        setMsgSuccess(ui->label_idMsg, "✅ 사용 가능한 아이디입니다.");
        m_idChecked = true;
    } else {
        setMsgError(ui->label_idMsg, "❌ " + message);
        m_idChecked = false;
    }
    updateSignupButtonState();
}

void LoginWidget::onPhoneCheckResponse(int status, QString message, bool isAvailable)
{
    ui->btnCheckPhone->setEnabled(true);
    if (isAvailable) {
        setMsgSuccess(ui->label_phoneMsg, "✅ 사용 가능한 전화번호입니다.");
        m_phoneChecked = true;
    } else {
        setMsgError(ui->label_phoneMsg, "❌ " + message);
        m_phoneChecked = false;
    }
    updateSignupButtonState();
}

// ── 메시지 헬퍼 ──
void LoginWidget::setMsgSuccess(QLabel *label, const QString &msg) {
    label->setText(msg);
    label->setStyleSheet("font-size:12px; color:#1565c0; padding-left:4px;");
}
void LoginWidget::setMsgError(QLabel *label, const QString &msg) {
    label->setText(msg);
    label->setStyleSheet("font-size:12px; color:#e53935; padding-left:4px;");
}
void LoginWidget::setMsgNeutral(QLabel *label, const QString &msg) {
    label->setText(msg);
    label->setStyleSheet("font-size:12px; color:#aaaaaa; padding-left:4px;");
}
