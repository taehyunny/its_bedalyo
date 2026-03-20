#pragma once
#include <QMainWindow>
#include "NetworkManager.h"
#include "LoginWidget.h"
#include "HomeWidget.h"

// ============================================================
// MainWindow
// 역할: 전체 화면 전환을 관리하는 최상위 컨테이너
//
// 구조:
//   - 창 크기 390x844 고정 (모바일 비율)
//   - QStackedWidget(stackedWidget)으로 각 화면을 페이지처럼 관리
//   - NetworkManager를 생성하고 각 Widget에 포인터로 주입
//
// 화면 추가 방법:
//   1. XxxWidget.h/cpp/ui 생성
//   2. 멤버 변수 추가: XxxWidget *m_xxxWidget;
//   3. 생성자에서: ui->stackedWidget->addWidget(m_xxxWidget);
//   4. 전환 시:    ui->stackedWidget->setCurrentWidget(m_xxxWidget);
// ============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 다른 Widget에서 화면 전환을 요청할 때 사용
    // 예: m_mainWindow->showHome();
    void showLogin();
    void showHome();

private slots:
    // LoginWidget::loginSuccess 수신 → HomeWidget으로 전환
    void onLoginSuccess(QString userName);

    // HomeWidget::logoutRequested 수신 → LoginWidget으로 복귀
    void onLogoutRequested();

private:
    Ui::MainWindow *ui;

    NetworkManager *m_network;       // 통신 전담 객체 (모든 Widget이 공유)
    LoginWidget    *m_loginWidget;   // 로그인/회원가입 화면
    HomeWidget     *m_homeWidget;    // 홈(가게 목록) 화면

    // ── 향후 화면 추가 시 여기에 멤버 변수 추가 ──
    // OrderWidget   *m_orderWidget;
    // MyPageWidget  *m_myPageWidget;
};
