#pragma once
#include <QWidget>
#include "NetworkManager.h" // 통신 전담 클래스

// ============================================================
// LoginWidget
// 역할: 로그인 / 회원가입 화면을 담당하는 Widget
//
// 구조:
//   - QStackedWidget(formStack)으로 로그인 탭 / 회원가입 탭 전환
//   - NetworkManager를 외부(MainWindow)에서 주입받아 사용
//     → 통신 객체를 직접 생성하지 않아 결합도가 낮음
//
// 시그널 흐름:
//   로그인 성공 → emit loginSuccess(userName)
//              → MainWindow::onLoginSuccess() 수신
//              → 다음 화면(HomeWidget 등)으로 전환
// ============================================================

// UI 클래스 전방 선언 (ui_loginwidget.h 자동 생성)
QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    // NetworkManager를 외부에서 주입받는 생성자
    // MainWindow에서 생성한 NetworkManager 포인터를 넘겨줌
    explicit LoginWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    // 로그인 성공 시 MainWindow로 전달
    // userName: 서버에서 받은 유저 이름
    void loginSuccess(QString userName);

private slots:
    // UI 버튼과 자동 연결 (on_버튼이름_clicked 네이밍 규칙)
    void on_loginButton_clicked();   // 로그인 버튼
    void on_signupButton_clicked();  // 회원가입 버튼

    // NetworkManager::onAuthResponse 시그널 수신
    // 로그인/회원가입 결과 처리
    void onAuthResponse(int status, QString message, QString userName);

private:
    Ui::LoginWidget *ui;       // UI 객체 (loginwidget.ui로부터 자동 생성)
    NetworkManager  *m_network; // 통신 객체 (외부 주입, 소유권 없음)
};
