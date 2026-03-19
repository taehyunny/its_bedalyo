#pragma once
#include <QMainWindow>
#include "NetworkManager.h" // 통신 전담 클래스
#include "LoginWidget.h"    // 로그인/회원가입 화면

// ============================================================
// MainWindow
// 역할: 전체 화면 전환을 관리하는 최상위 컨테이너
//
// 구조:
//   - QStackedWidget(stackedWidget)으로 각 화면을 페이지처럼 관리
//   - NetworkManager를 생성하고 각 Widget에 포인터로 주입
//   - 로그인 성공 시 다음 화면(HomeWidget 등)으로 전환
//
// 화면 추가 방법:
//   1. HomeWidget.h/cpp/ui 생성
//   2. 멤버 변수 추가: HomeWidget *m_homeWidget;
//   3. 생성자에서: ui->stackedWidget->addWidget(m_homeWidget);
//   4. 전환 시: ui->stackedWidget->setCurrentWidget(m_homeWidget);
// ============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // NetworkManager::onConnected 수신 → 상태 라벨 "✅ 서버 연결됨" 표시
    void onConnected();

    // LoginWidget::loginSuccess 수신 → 다음 화면으로 전환
    // 현재는 상태 라벨 업데이트만 (HomeWidget 구현 후 화면 전환으로 교체)
    void onLoginSuccess(QString userName);

private:
    Ui::MainWindow *ui;         // mainwindow.ui 기반 UI 객체
    NetworkManager *m_network;  // 통신 전담 객체 (모든 Widget이 공유)
    LoginWidget    *m_loginWidget; // 로그인/회원가입 화면

    // ── 향후 화면 추가 시 여기에 멤버 변수 추가 ──
    // HomeWidget    *m_homeWidget;
    // OrderWidget   *m_orderWidget;
    // MyPageWidget  *m_myPageWidget;
};
