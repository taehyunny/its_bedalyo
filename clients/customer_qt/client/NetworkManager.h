#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include "Global_protocol.h"
#include "AccountDTO.h"

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &ip, quint16 port);

    // 인증
    void sendLogin(const LoginReqDTO &dto);
    void sendSignup(const SignupReqDTO &dto);

    // 중복확인 (DTO 정의 완료 → 바로 사용 가능)
    void sendIdCheck(const AuthCheckReqDTO &dto);    // REQ_AUTH_CHECK = 1040
    void sendPhoneCheck(const PhoneCheckReqDTO &dto); // REQ_PHONE_CHECK = 1042

signals:
    void onConnected();

    // 로그인/회원가입 응답
    void onAuthResponse(int status, QString message, QString userName);

    // 아이디 중복확인 응답 (RES_AUTH_CHECK = 1041)
    void onIdCheckResponse(int status, QString message, bool isAvailable);

    // 전화번호 중복확인 응답 (RES_PHONE_CHECK = 1043)
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

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
