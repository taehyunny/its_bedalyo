#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include "Global_protocol.h"
#include "BaseDTO.h"     // ← StoreDTO.h 보다 먼저!
#include "AccountDTO.h"
#include "StoreDTO.h"    // ← BaseDTO.h 다음에

// Qt용 변환 구조체
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
    double  rating         = 0.0;
    int     reviewCount    = 0;
    int     minOrderAmount = 0;
    int     deliveryFee    = 0;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &ip, quint16 port);

    // ── 인증 관련 ──
    void sendLogin(const LoginReqDTO &dto);
    void sendSignup(const SignupReqDTO &dto);
    void sendIdCheck(const AuthCheckReqDTO &dto);
    void sendPhoneCheck(const PhoneCheckReqDTO &dto);

    // ── 카테고리별 가게 목록 요청 (REQ_STORE_LIST = 2000) ──
    void sendStoreListRequest(int categoryId);

signals:
    void onConnected();

    // ── 로그인 응답 (서버가 userName, address 채워서 줌) ──
    void onLoginResponse(int status, QString message, QString userName, QString address);

    // ── 회원가입 응답 (서버는 성공/실패만 알려줌) ──
    void onSignupResponse(int status, QString message);

    // ── 아이디/전화번호 중복확인 응답 ──
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

    // ── 메인 홈 데이터 수신 (RES_CATEGORY) - 연결 시 서버 자동 push ──
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

    // ── 카테고리별 가게 목록 수신 (RES_STORE_LIST = 2001, menucategori에서 사용) ──
    void onStoreListReceived(QList<TopStoreInfoQt> stores);

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
