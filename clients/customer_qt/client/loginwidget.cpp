#include "LoginWidget.h"
#include "ui_loginwidget.h"
#include "AccountDTO.h"
#include "UserSession.h"
#include <QMessageBox>

// ── 탭 스타일 상수 ──
static const QString TAB_ACTIVE =
    "color:#1565c0; border-bottom:2.5px solid #1565c0; background:transparent;"
    "font-size:16px; font-weight:bold; padding:10px 0px;"
    "border-top:none; border-left:none; border-right:none;";

static const QString TAB_INACTIVE =
    "color:#bbbbbb; border-bottom:2.5px solid transparent; background:transparent;"
    "font-size:16px; font-weight:bold; padding:10px 0px;"
    "border-top:none; border-left:none; border-right:none;";

// ── 비밀번호 정규식: 최소 8자, 영문+숫자+특수문자 각 1개 이상 ──
const QRegularExpression LoginWidget::PW_REGEX(
    "^(?=.*[A-Za-z])(?=.*\\d)(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?]).{8,}$"
);

// ── 전화번호 정규식: 숫자만 10~11자리 ──
const QRegularExpression LoginWidget::PHONE_REGEX("^[0-9]{10,11}$");

// ============================================================
// 생성자
// ============================================================
LoginWidget::LoginWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // 초기 탭: 로그인
    ui->formStack->setCurrentIndex(0);

    // ── 탭 전환 ──
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

    // ── 비밀번호 실시간 검증 연결 ──
    connect(ui->signupPwEdit,        &QLineEdit::textChanged,
            this, &LoginWidget::onPwChanged);
    connect(ui->signupPwConfirmEdit, &QLineEdit::textChanged,
            this, &LoginWidget::onPwConfirmChanged);

    // ── 아이디 변경 시 중복확인 플래그 초기화 ──
    connect(ui->signupIdEdit, &QLineEdit::textChanged, this, [this]() {
        m_idChecked = false;
        setMsgNeutral(ui->label_idMsg, "");
        updateSignupButtonState();
    });

    // ── 전화번호 실시간 형식 검증 + 중복확인 플래그 초기화 ──
    connect(ui->phoneEdit, &QLineEdit::textChanged,
            this, &LoginWidget::onPhoneChanged);

    // ── 나머지 입력 필드 변경 시 버튼 상태 재검사 ──
    connect(ui->nameEdit,    &QLineEdit::textChanged,
            this, &LoginWidget::onSignupFieldChanged);
    connect(ui->addressEdit, &QLineEdit::textChanged,
            this, &LoginWidget::onSignupFieldChanged);

    // ── 서버 응답 연결 (로그인/회원가입 분리) ──
    connect(m_network, &NetworkManager::onLoginResponse,
            this, &LoginWidget::onLoginResponse);
    connect(m_network, &NetworkManager::onSignupResponse,
            this, &LoginWidget::onSignupResponse);
    connect(m_network, &NetworkManager::onIdCheckResponse,
            this, &LoginWidget::onIdCheckResponse);
    connect(m_network, &NetworkManager::onPhoneCheckResponse,
            this, &LoginWidget::onPhoneCheckResponse);
}

LoginWidget::~LoginWidget() { delete ui; }

// ============================================================
// 로그인 버튼
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
    dto.role     = 0;
    m_network->sendLogin(dto);
}

// ============================================================
// 아이디 중복확인 버튼 (REQ_AUTH_CHECK = 1040)
// ============================================================
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

// ============================================================
// 전화번호 중복확인 버튼 (REQ_PHONE_CHECK = 1042)
// role=0 (고객) 함께 전송
// ============================================================
void LoginWidget::on_btnCheckPhone_clicked()
{
    if (ui->phoneEdit->text().isEmpty()) {
        setMsgError(ui->label_phoneMsg, "전화번호를 먼저 입력해주세요.");
        return;
    }
    if (!m_phoneValid) {
        setMsgError(ui->label_phoneMsg, "❌ 숫자만 입력해주세요. (예: 01012345678)");
        return;
    }

    PhoneCheckReqDTO dto;
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.role        = 0; // 고객
    m_network->sendPhoneCheck(dto);

    setMsgNeutral(ui->label_phoneMsg, "확인 중...");
    ui->btnCheckPhone->setEnabled(false);
}

// ============================================================
// 전화번호 형식 실시간 검증
// 조건: 숫자만 10~11자리 (하이픈 없이 입력)
// ============================================================
void LoginWidget::onPhoneChanged()
{
    QString phone = ui->phoneEdit->text();

    // 전화번호 변경 시 중복확인 플래그 초기화
    m_phoneChecked = false;

    if (phone.isEmpty()) {
        setMsgNeutral(ui->label_phoneMsg, "");
        m_phoneValid = false;
    } else if (PHONE_REGEX.match(phone).hasMatch()) {
        setMsgSuccess(ui->label_phoneMsg, "✅ 올바른 전화번호 형식입니다.");
        m_phoneValid = true;
    } else {
        setMsgError(ui->label_phoneMsg, "❌ 숫자만 입력해주세요. (예: 01012345678)");
        m_phoneValid = false;
    }

    // 중복확인 버튼 — 형식이 맞을 때만 활성화
    ui->btnCheckPhone->setEnabled(m_phoneValid);
    updateSignupButtonState();
}

// ============================================================
// 비밀번호 형식 실시간 검증
// 조건: 최소 8자, 영문 + 숫자 + 특수문자 포함
// ============================================================
void LoginWidget::onPwChanged()
{
    QString pw = ui->signupPwEdit->text();

    if (pw.isEmpty()) {
        setMsgNeutral(ui->label_pwMsg, "");
        m_pwValid = false;
    } else if (PW_REGEX.match(pw).hasMatch()) {
        setMsgSuccess(ui->label_pwMsg, "✅ 사용 가능한 비밀번호입니다.");
        m_pwValid = true;
    } else {
        setMsgError(ui->label_pwMsg, "❌ 최소 8자, 영문+숫자+특수문자를 포함해야 합니다.");
        m_pwValid = false;
    }

    // 비밀번호가 바뀌면 확인란도 다시 체크
    onPwConfirmChanged();
}

// ============================================================
// 비밀번호 확인 실시간 일치 여부
// ============================================================
void LoginWidget::onPwConfirmChanged()
{
    QString pw        = ui->signupPwEdit->text();
    QString pwConfirm = ui->signupPwConfirmEdit->text();

    if (pwConfirm.isEmpty()) {
        if (m_pwValid)
            setMsgSuccess(ui->label_pwMsg, "✅ 사용 가능한 비밀번호입니다.");
        m_pwMatched = false;
    } else if (!m_pwValid) {
        setMsgError(ui->label_pwMsg, "❌ 최소 8자, 영문+숫자+특수문자를 포함해야 합니다.");
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

// ============================================================
// 입력 필드 변경 시 회원가입 버튼 상태 재검사
// ============================================================
void LoginWidget::onSignupFieldChanged()
{
    updateSignupButtonState();
}

// ============================================================
// 회원가입 버튼 활성화 조건
// ============================================================
void LoginWidget::updateSignupButtonState()
{
    bool allFilled = !ui->signupIdEdit->text().isEmpty()
                  && !ui->signupPwEdit->text().isEmpty()
                  && !ui->signupPwConfirmEdit->text().isEmpty()
                  && !ui->nameEdit->text().isEmpty()
                  && !ui->phoneEdit->text().isEmpty()
                  && !ui->addressEdit->text().isEmpty();

    bool canSignup = allFilled
                  && m_idChecked
                  && m_phoneChecked
                  && m_phoneValid
                  && m_pwValid
                  && m_pwMatched;

    ui->signupButton->setEnabled(canSignup);
}

// ============================================================
// 회원가입 요청 전송
// ============================================================
void LoginWidget::on_signupButton_clicked()
{
    if (!m_idChecked || !m_phoneChecked || !m_pwValid || !m_pwMatched) return;

    SignupReqDTO dto;
    dto.userId      = ui->signupIdEdit->text().toStdString();
    dto.password    = ui->signupPwEdit->text().toStdString();
    dto.userName    = ui->nameEdit->text().toStdString();
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.address     = ui->addressEdit->text().toStdString();
    dto.role        = 0; // 고객

    m_network->sendSignup(dto);
}

// ============================================================
// 로그인 응답 처리 (RES_LOGIN)
// 서버가 userName, address 채워서 줌
// ============================================================
void LoginWidget::onLoginResponse(int status, QString message, QString userName, QString address, QString phoneNumber)
{
    qDebug() << "[LoginResponse] status:" << status
             << "userName:" << userName
             << "address:" << address        // ← 이게 뭐가 오는지 확인
             << "phoneNumber:" << phoneNumber;

    if (status == 200) {
        UserSession::instance().set(userName, address, ui->idEdit->text(), phoneNumber);
        emit loginSuccess();
    } else {
        QMessageBox::warning(this, "로그인 실패", message);
    }
}

// ============================================================
// 회원가입 응답 처리 (RES_SIGNUP)
// 서버는 성공/실패만 알려줌 → 클라 입력값으로 UserSession 저장 후 바로 홈 전환
// ============================================================
void LoginWidget::onSignupResponse(int status, QString message)
{
    if (status == 200) {
        // 서버 응답 대신 직접 입력했던 값으로 UserSession 저장
        UserSession::instance().set(
            ui->nameEdit->text(),
            ui->addressEdit->text(),
            ui->signupIdEdit->text(),
            ui->phoneEdit->text()
        );
        // 로그인한 것과 동일하게 홈 화면으로 전환
        emit loginSuccess();
    } else {
        QMessageBox::warning(this, "회원가입 실패", message);
    }
}

// ── 아이디 중복확인 응답 (RES_AUTH_CHECK) ──
void LoginWidget::onIdCheckResponse(int status, QString message, bool isAvailable)
{
    Q_UNUSED(status)
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

// ── 전화번호 중복확인 응답 (RES_PHONE_CHECK) ──
void LoginWidget::onPhoneCheckResponse(int status, QString message, bool isAvailable)
{
    Q_UNUSED(status)
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

// ── 메시지 라벨 스타일 헬퍼 ──
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
