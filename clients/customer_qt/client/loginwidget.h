#pragma once
#include <QWidget>
#include <QLabel>
#include <QRegularExpression>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

// ============================================================
// LoginWidget - 로그인 / 회원가입 화면
//
// - 탭 전환: 로그인 ↔ 회원가입
// - 회원가입: 아이디/전화번호 중복확인, 비밀번호 검증, 조건부 버튼 활성화
// - 로그인 성공  → UserSession에 서버 응답값 저장 → loginSuccess 시그널 emit
// - 회원가입 성공 → UserSession에 클라 입력값 저장 → loginSuccess 시그널 emit (바로 홈 전환)
// ============================================================
class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    // 로그인/회원가입 성공 시 MainWindow에 알림 → 홈 화면으로 전환
    void loginSuccess();

private slots:
    // ── 로그인 ──
    void on_loginButton_clicked();

    // ── 회원가입 ──
    void on_btnCheckId_clicked();       // 아이디 중복확인
    void on_btnCheckPhone_clicked();    // 전화번호 중복확인
    void on_signupButton_clicked();     // 회원가입 버튼

    // ── 실시간 검증 ──
    void onPwChanged();                 // 비밀번호 형식 검증
    void onPwConfirmChanged();          // 비밀번호 일치 확인
    void onSignupFieldChanged();        // 회원가입 버튼 활성화 조건 재검사

    // ── 서버 응답 슬롯 ──
    void onLoginResponse(int status, QString message, QString userName, QString address, QString phoneNumber);
    void onSignupResponse(int status, QString message); // ← 회원가입 전용 (분리)
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

private:
    Ui::LoginWidget *ui;
    NetworkManager  *m_network;

    // ── 회원가입 조건 플래그 ──
    bool m_idChecked    = false;  // 아이디 중복확인 완료
    bool m_phoneChecked = false;  // 전화번호 중복확인 완료
    bool m_pwValid      = false;  // 비밀번호 형식 충족
    bool m_pwMatched    = false;  // 비밀번호 일치

    // ── 비밀번호 조건 정규식 (최소 8자, 영문+숫자+특수문자) ──
    static const QRegularExpression PW_REGEX;

    // ── 회원가입 버튼 활성화 조건 검사 ──
    void updateSignupButtonState();

    // ── 메시지 라벨 스타일 헬퍼 ──
    void setMsgSuccess(QLabel *label, const QString &msg);
    void setMsgError(QLabel *label, const QString &msg);
    void setMsgNeutral(QLabel *label, const QString &msg);
};
