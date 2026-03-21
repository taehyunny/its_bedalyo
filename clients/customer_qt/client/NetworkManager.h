#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include "Global_protocol.h"
#include "BaseDTO.h"     // ← StoreDTO.h 보다 먼저!
#include "AccountDTO.h"
#include "StoreDTO.h"    // ← BaseDTO.h 다음에
// Qt용 변환 구조체 (HomeWidget에서 사용)
struct CategoryInfoQt {
    int     id;
    QString name;
    QString iconPath;
};

struct TopStoreInfoQt {
    int     storeId;
    QString storeName;
    QString category;
    QString iconPath;
    QString deliveryTimeRange;
    double  rating       = 0.0;
    int     reviewCount  = 0;
    int     minOrderAmount = 0;
    int     deliveryFee  = 0;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &ip, quint16 port);
    void sendLogin(const LoginReqDTO &dto);
    void sendSignup(const SignupReqDTO &dto);
    void sendIdCheck(const AuthCheckReqDTO &dto);
    void sendPhoneCheck(const PhoneCheckReqDTO &dto);

signals:
    void onConnected();

    // 로그인/회원가입 응답
    void onAuthResponse(int status, QString message, QString userName);

    // 아이디/전화번호 중복확인 응답
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

    // 메인 화면 데이터 수신 (RES_CATEGORY = 2102)
    // 서버 연결 시 자동으로 push됨
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *m_socket;
    QByteArray  m_buffer;

    void sendPacket(CmdID cmdId, const nlohmann::json &body);
    void processPacket(CmdID cmdId, const QByteArray &body);
};
