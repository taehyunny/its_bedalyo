#pragma once
#include <QWidget>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWidget; }
QT_END_NAMESPACE

// ============================================================
// SettingsWidget - 설정 화면
//
// [ 기능 ]
// - 로그아웃 : 확인 팝업 → logoutRequested 시그널 emit
// - 회원탈퇴 : 비밀번호 확인 팝업 → REQ_WITHDRAW(1070) 전송 (TODO)
// ============================================================
class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~SettingsWidget();

signals:
    void backRequested();
    void logoutRequested();

private slots:
    void on_btnBack_clicked();
    void on_btnLogout_clicked();
    void on_btnWithdraw_clicked();

private:
    Ui::SettingsWidget *ui;
    NetworkManager     *m_network;
};
