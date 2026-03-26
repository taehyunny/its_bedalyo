#pragma once
#include <QWidget>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyPageWidget; }
QT_END_NAMESPACE

class MyPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyPageWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~MyPageWidget();

    void loadData();
    void setStats(int reviewCount, int helpCount, int favoriteCount);

signals:
    void homeRequested();
    void searchRequested();
    void favoriteRequested();
    void orderListRequested();
    void policyRequested();    // 약관 및 정책 화면으로
    void settingsRequested();  // 설정 화면으로
    void logoutRequested();    // 설정 화면에서 emit → MainWindow 전달용
    void addressRequested();

private slots:
    void on_btnDetail_clicked();
    void on_btnAddress_clicked();
    void on_btnFavorite_clicked();
    void on_btnWow_clicked();
    void on_btnFaq_clicked();
    void on_btnPolicy_clicked();
    void on_btnSettings_clicked();

    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

private:
    Ui::MyPageWidget *ui;
    NetworkManager   *m_network;

    void updateProfileUI();
    static QString maskPhoneNumber(const QString &phone);
};
