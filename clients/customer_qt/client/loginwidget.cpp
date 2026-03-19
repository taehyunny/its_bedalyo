#include "LoginWidget.h"
#include "ui_loginwidget.h" // Qt가 loginwidget.ui로부터 자동 생성하는 헤더
#include "AccountDTO.h"     // LoginReqDTO, SignupReqDTO (공용 DTO)
#include <QMessageBox>

// ============================================================
// 생성자
// UI 초기화 + 탭 전환 로직 + 서버 응답 수신 연결
// ============================================================
LoginWidget::LoginWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_network(network) // 외부에서 주입된 NetworkManager 포인터 저장
{
    ui->setupUi(this); // loginwidget.ui 기반으로 위젯 초기화

    // ── 탭 전환: 로그인 탭 클릭 ──
    // formStack의 0번 페이지(loginPage)로 전환
    // tabLogin 활성화 스타일 / tabSignup 비활성화 스타일 적용
    connect(ui->tabLogin, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(0);
        ui->tabLogin->setStyleSheet("color: #1565c0; border-bottom: 2px solid #1565c0; background: transparent; font-size: 15px; font-weight: bold; padding: 8px 0px; border-top: none; border-left: none; border-right: none;");
        ui->tabSignup->setStyleSheet("color: #aaaaaa; border-bottom: 2px solid transparent; background: transparent; font-size: 15px; font-weight: bold; padding: 8px 0px; border-top: none; border-left: none; border-right: none;");
        // TODO: 스타일 문자열을 상수로 분리 권장
        // const QString ACTIVE_STYLE   = "color: #1565c0; ...";
        // const QString INACTIVE_STYLE = "color: #aaaaaa; ...";
    });

    // ── 탭 전환: 회원가입 탭 클릭 ──
    // formStack의 1번 페이지(signupPage)로 전환
    connect(ui->tabSignup, &QPushButton::clicked, this, [this]() {
        ui->formStack->setCurrentIndex(1);
        ui->tabSignup->setStyleSheet("color: #1565c0; border-bottom: 2px solid #1565c0; background: transparent; font-size: 15px; font-weight: bold; padding: 8px 0px; border-top: none; border-left: none; border-right: none;");
        ui->tabLogin->setStyleSheet("color: #aaaaaa; border-bottom: 2px solid transparent; background: transparent; font-size: 15px; font-weight: bold; padding: 8px 0px; border-top: none; border-left: none; border-right: none;");
    });

    // ── 서버 인증 응답 수신 연결 ──
    // NetworkManager가 onAuthResponse를 emit하면 이 클래스의 onAuthResponse 슬롯 호출
    connect(m_network, &NetworkManager::onAuthResponse,
            this, &LoginWidget::onAuthResponse);
}

LoginWidget::~LoginWidget()
{
    delete ui;
    // m_network는 MainWindow 소유 → 여기서 삭제하지 않음
}

// ============================================================
// 로그인 버튼 클릭 슬롯
// 입력값 검증 → LoginReqDTO 구성 → NetworkManager::sendLogin 호출
// ============================================================
void LoginWidget::on_loginButton_clicked()
{
    // 입력값 검증
    if (ui->idEdit->text().isEmpty() || ui->pwEdit->text().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "아이디와 비밀번호를 입력해주세요.");
        return;
    }

    // DTO 구성 및 전송 (REQ_LOGIN = 1010)
    LoginReqDTO dto;
    dto.userId   = ui->idEdit->text().toStdString();
    dto.password = ui->pwEdit->text().toStdString();
    m_network->sendLogin(dto);
}

// ============================================================
// 회원가입 버튼 클릭 슬롯
// 입력값 검증 → SignupReqDTO 구성 → NetworkManager::sendSignup 호출
// ============================================================
void LoginWidget::on_signupButton_clicked()
{
    // 모든 필드 입력 여부 검증
    if (ui->signupIdEdit->text().isEmpty() || ui->signupPwEdit->text().isEmpty()
        || ui->nameEdit->text().isEmpty() || ui->phoneEdit->text().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "모든 항목을 입력해주세요.");
        return;
    }

    // DTO 구성 및 전송 (REQ_SIGNUP = 1020)
    SignupReqDTO dto;
    dto.userId      = ui->signupIdEdit->text().toStdString();
    dto.password    = ui->signupPwEdit->text().toStdString();
    dto.userName    = ui->nameEdit->text().toStdString();
    dto.phoneNumber = ui->phoneEdit->text().toStdString();
    dto.role        = 0; // 0: 고객, 1: 사장님, 2: 관리자/라이더
    m_network->sendSignup(dto);
}

// ============================================================
// 서버 인증 응답 수신 슬롯
// NetworkManager::onAuthResponse 시그널로부터 호출됨
//
// TODO: 현재는 status==200 && userName 유무로만 로그인/회원가입 구분
//       향후 현재 탭 상태(m_currentTab)를 멤버 변수로 관리하여 명확히 구분 필요
// ============================================================
void LoginWidget::onAuthResponse(int status, QString message, QString userName)
{
    // 로그인 성공: status 200 + userName 있음
    if (status == 200 && !userName.isEmpty()) {
        emit loginSuccess(userName); // MainWindow로 전달 → 화면 전환
    } else {
        // 실패 또는 회원가입 성공(userName 없는 경우) → 팝업 표시
        QString title = (status == 200) ? "성공" : "실패";
        QMessageBox::information(this, title, message);
    }
}
