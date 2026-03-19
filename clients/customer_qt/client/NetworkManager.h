#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include "Global_protocol.h" // CmdID enum, PacketHeader 구조체 (공용 파일)
#include "AccountDTO.h"      // LoginReqDTO, SignupReqDTO, AuthResDTO (공용 DTO)

// ============================================================
// NetworkManager
// 역할: 서버와의 모든 TCP 통신을 전담하는 클래스
//
// 설계 원칙:
//   - UI(Widget)와 통신 로직을 완전히 분리
//   - 송신: DTO → JSON 직렬화 → PacketHeader + Body 조립 → 전송
//   - 수신: 버퍼에 누적 → 완전한 패킷 감지 → JSON 역직렬화 → 시그널 emit
//
// 사용 방법:
//   NetworkManager *m_network = new NetworkManager(this);
//   m_network->connectToServer("10.10.10.123", 8080);
//   m_network->sendLogin(dto);  // 로그인 요청
//   connect(m_network, &NetworkManager::onAuthResponse, this, &YourWidget::onAuthResponse);
// ============================================================
class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    // 서버에 TCP 연결 시도
    // 연결 성공 시 onConnected() 시그널 발생
    void connectToServer(const QString &ip, quint16 port);

    // 로그인 요청 전송 (REQ_LOGIN = 1010)
    // 서버 응답은 onAuthResponse() 시그널로 수신
    void sendLogin(const LoginReqDTO &dto);

    // 회원가입 요청 전송 (REQ_SIGNUP = 1020)
    // 서버 응답은 onAuthResponse() 시그널로 수신
    void sendSignup(const SignupReqDTO &dto);

    // ── 향후 API 추가 시 여기에 sendXxx() 함수 추가 ──
    // void sendOrderCreate(const OrderCreateReqDTO &dto);
    // void sendStoreList();

signals:
    // 로그인/회원가입 응답 수신 시 발생
    // status  : 200(성공), 401(실패), 409(중복)
    // message : 서버에서 보낸 메시지 (UI 팝업 표시용)
    // userName: 로그인 성공 시 유저 이름 (회원가입 시 빈 문자열일 수 있음)
    void onAuthResponse(int status, QString message, QString userName);

    // 서버 TCP 연결 성공 시 발생
    // MainWindow에서 수신하여 상태 라벨 업데이트
    void onConnected();

private slots:
    // QTcpSocket::connected 시그널 수신 → onConnected emit
    void handleConnected();

    // 데이터 수신 시 호출 — 버퍼에 누적하여 완전한 패킷 단위로 처리
    void handleReadyRead();

    // 소켓 에러 발생 시 호출 (연결 실패, 연결 끊김 등)
    void handleError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *m_socket; // TCP 소켓 객체
    QByteArray  m_buffer; // 수신 버퍼 (패킷이 여러 조각으로 올 수 있어 누적 필요)

    // 내부 전송 함수: PacketHeader + JSON Body 조립 후 소켓으로 전송
    void sendPacket(CmdID cmdId, const nlohmann::json &body);

    // 내부 수신 처리: cmdId에 따라 적절한 DTO로 역직렬화 후 시그널 emit
    void processPacket(CmdID cmdId, const QByteArray &body);
};
