#pragma once
#include <QMainWindow>
#include "NetworkManager.h"
#include "LoginWidget.h"
#include "HomeWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// ============================================================
// MainWindow - 앱 최상위 컨테이너
//
// - QStackedWidget으로 화면 전환 관리
// - NetworkManager를 생성해 각 Widget에 주입
// - 화면 추가 시: Widget 생성 → stackedWidget에 추가 → 시그널 연결
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showLogin();
    void showHome();

private slots:
    // 로그인/회원가입 성공 → 홈 화면으로 전환
    void onLoginSuccess();

    // 로그아웃 → 로그인 화면으로 복귀
    void onLogoutRequested();

private:
    Ui::MainWindow *ui;
    NetworkManager *m_network;      // 통신 전담 (모든 Widget이 공유)
    LoginWidget    *m_loginWidget;
    HomeWidget     *m_homeWidget;
};
