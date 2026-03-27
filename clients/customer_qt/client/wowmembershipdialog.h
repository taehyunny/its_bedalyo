#ifndef WOWMEMBERSHIPDIALOG_H
#define WOWMEMBERSHIPDIALOG_H

#include <QDialog>
#include <QString>

// NetworkManager가 있다는 것을 미리 알려줌
class NetworkManager;

struct WowUserData {
    QString userName;
    QString membershipStatus;
};

namespace Ui { class WowMembershipDialog; }

class WowMembershipDialog : public QDialog
{
    Q_OBJECT

public:
    // 생성자 인자에 NetworkManager*를 추가함
    explicit WowMembershipDialog(NetworkManager *network, QWidget *parent = nullptr);
    ~WowMembershipDialog();

    void setUserInfo(const WowUserData &data);

private slots:
    void on_btnBack_clicked();
    void on_btnApply_clicked();
    void on_btnTerminate_clicked();
    // ✅ 서버 응답을 처리할 슬롯 추가
    void handleResponse(int status, QString message);

private:
    Ui::WowMembershipDialog *ui;
    // ✅ 네트워크 매니저를 저장할 변수 추가
    NetworkManager *m_network;
};

#endif