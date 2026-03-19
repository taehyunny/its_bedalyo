#include "NetworkManager.h"
#include "json.hpp" // nlohmann json (common/include/json.hpp)
#include <QDebug>
#include <cstring>  // memcpy

// ============================================================
// 생성자
// QTcpSocket 생성 및 시그널-슬롯 연결
// ============================================================
NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    // 서버 연결 성공 시
    connect(m_socket, &QTcpSocket::connected,
            this, &NetworkManager::handleConnected);

    // 데이터 수신 가능 시 (readyRead는 소켓 버퍼에 데이터가 도착할 때마다 호출)
    connect(m_socket, &QTcpSocket::readyRead,
            this, &NetworkManager::handleReadyRead);

    // 소켓 에러 발생 시
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &NetworkManager::handleError);
}

// ============================================================
// 서버 연결 시도
// ============================================================
void NetworkManager::connectToServer(const QString &ip, quint16 port)
{
    m_socket->connectToHost(ip, port);
    // 결과는 handleConnected() 또는 handleError()로 비동기 수신
}

// ============================================================
// 로그인 요청 전송
// LoginReqDTO → JSON → 패킷 조립 → 전송
// ============================================================
void NetworkManager::sendLogin(const LoginReqDTO &dto)
{
    nlohmann::json j = dto; // NLOHMANN_DEFINE_TYPE_INTRUSIVE 매크로로 자동 직렬화
    sendPacket(CmdID::REQ_LOGIN, j);
}

// ============================================================
// 회원가입 요청 전송
// SignupReqDTO → JSON → 패킷 조립 → 전송
// ============================================================
void NetworkManager::sendSignup(const SignupReqDTO &dto)
{
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_SIGNUP, j);
}

// ============================================================
// 패킷 조립 및 전송 (내부 함수)
//
// 패킷 구조:
//   [ PacketHeader(8byte) ][ JSON Body(가변) ]
//   - signature : 0x4543 ('E','C') 고정
//   - cmdId     : 요청 종류 식별
//   - bodySize  : JSON 바이트 크기
// ============================================================
void NetworkManager::sendPacket(CmdID cmdId, const nlohmann::json &body)
{
    std::string bodyStr = body.dump(); // JSON → 문자열 직렬화

    // 헤더 구성
    PacketHeader header;
    header.signature = 0x4543;
    header.cmdId     = cmdId;
    header.bodySize  = static_cast<uint32_t>(bodyStr.size());

    // 헤더 + 바디 합쳐서 하나의 QByteArray로 전송
    QByteArray packet;
    packet.append(reinterpret_cast<const char*>(&header), sizeof(PacketHeader));
    packet.append(bodyStr.c_str(), bodyStr.size());

    m_socket->write(packet);
    m_socket->flush(); // 즉시 전송 보장
}

// ============================================================
// 데이터 수신 처리 (내부 슬롯)
//
// 주의: TCP는 스트림 기반이라 패킷이 여러 조각으로 올 수 있음
//       → m_buffer에 누적 후 완전한 패킷이 됐을 때만 처리
// ============================================================
void NetworkManager::handleReadyRead()
{
    // 새로 도착한 데이터를 버퍼에 추가
    m_buffer.append(m_socket->readAll());

    // 버퍼에 완전한 패킷이 있는 동안 계속 처리
    while (m_buffer.size() >= static_cast<int>(sizeof(PacketHeader)))
    {
        // 헤더 읽기 (아직 버퍼에서 제거 안 함)
        PacketHeader header;
        std::memcpy(&header, m_buffer.constData(), sizeof(PacketHeader));

        // 시그니처 검증 — 잘못된 데이터 수신 방어
        if (header.signature != 0x4543) {
            qWarning() << "잘못된 시그니처, 버퍼 초기화";
            m_buffer.clear();
            return;
        }

        // 바디가 아직 전부 안 왔으면 대기 (다음 readyRead 때 재시도)
        int totalSize = sizeof(PacketHeader) + header.bodySize;
        if (m_buffer.size() < totalSize) break;

        // 완전한 패킷 → 바디 추출 후 버퍼에서 제거
        QByteArray body = m_buffer.mid(sizeof(PacketHeader), header.bodySize);
        m_buffer.remove(0, totalSize);

        // 패킷 처리
        processPacket(header.cmdId, body);
    }
}

// ============================================================
// 패킷 처리 (내부 함수)
// cmdId에 따라 적절한 DTO로 역직렬화 후 시그널 emit
//
// 향후 새 API 추가 시 여기에 if/else if 블록 추가:
//   else if (cmdId == CmdID::RES_ORDER_CREATE) { ... }
// ============================================================
void NetworkManager::processPacket(CmdID cmdId, const QByteArray &body)
{
    try {
        nlohmann::json j = nlohmann::json::parse(body.constData());

        // 로그인(1011) 또는 회원가입(1021) 응답
        if (cmdId == CmdID::RES_LOGIN || cmdId == CmdID::RES_SIGNUP) {
            AuthResDTO dto = j.get<AuthResDTO>(); // JSON → DTO 역직렬화
            emit onAuthResponse(dto.status,
                                QString::fromStdString(dto.message),
                                QString::fromStdString(dto.userName));
        }

        // ── 향후 응답 처리 추가 ──
        // else if (cmdId == CmdID::RES_STORE_LIST) { ... }
        // else if (cmdId == CmdID::RES_ORDER_CREATE) { ... }

    } catch (const std::exception &e) {
        qWarning() << "JSON 파싱 오류:" << e.what();
    }
}

// ============================================================
// 서버 연결 성공 슬롯
// ============================================================
void NetworkManager::handleConnected()
{
    qDebug() << "서버 연결 성공!";
    emit onConnected(); // MainWindow의 상태 라벨 업데이트용
}

// ============================================================
// 소켓 에러 슬롯
// ============================================================
void NetworkManager::handleError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    qWarning() << "소켓 에러:" << m_socket->errorString();
    // TODO: 재연결 로직 추가 (QTimer로 일정 시간 후 재시도)
}
