#pragma once
#include <QWidget>
#include <QLabel>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginSuccess(QString userName, QString address);

private slots:
    void on_loginButton_clicked();
    void on_btnCheckId_clicked();
    void on_btnCheckPhone_clicked();
    void on_signupButton_clicked();
    void onPwConfirmChanged();

    // 서버 응답 — isAvailable 파라미터 추가
    void onAuthResponse(int status, QString message, QString userName);
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

private:
    Ui::LoginWidget *ui;
    NetworkManager  *m_network;

    bool m_idChecked    = false;
    bool m_phoneChecked = false;
    bool m_pwMatched    = false;

    void updateSignupButtonState();
    void setMsgSuccess(QLabel *label, const QString &msg);
    void setMsgError(QLabel *label, const QString &msg);
    void setMsgNeutral(QLabel *label, const QString &msg);
};
