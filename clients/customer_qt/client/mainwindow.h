#pragma once
#include <QMainWindow>
#include "NetworkManager.h"
#include "LoginWidget.h"
#include "HomeWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showLogin();
    void showHome();

private slots:
    void onLoginSuccess(QString userName, QString address); // address 추가
    void onLogoutRequested();

private:
    Ui::MainWindow *ui;
    NetworkManager *m_network;
    LoginWidget    *m_loginWidget;
    HomeWidget     *m_homeWidget;
};
