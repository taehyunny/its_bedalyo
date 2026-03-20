#include "NetworkManager.h"
#include "json.hpp"
#include <QDebug>
#include <cstring>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected,
            this, &NetworkManager::handleConnected);

    connect(m_socket, &QTcpSocket::readyRead,
            this, &NetworkManager::handleReadyRead);

    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &NetworkManager::handleError);

    // ── 연결 끊김 감지 추가 ──
    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        qWarning() << "[NetworkManager] 서버 연결 끊김";
    });
}

void NetworkManager::connectToServer(const QString &ip, quint16 port)
{
    qDebug() << "[NetworkManager] 서버 연결 시도:" << ip << ":" << port;
    m_socket->connectToHost(ip, port);
}

void NetworkManager::sendLogin(const LoginReqDTO &dto)
{
    qDebug() << "[NetworkManager] 로그인 요청 -" << QString::fromStdString(dto.userId);
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_LOGIN, j);
}

void NetworkManager::sendSignup(const SignupReqDTO &dto)
{
    qDebug() << "[NetworkManager] 회원가입 요청 -" << QString::fromStdString(dto.userId)
        << "role:" << dto.role;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_SIGNUP, j);
}

void NetworkManager::sendPacket(CmdID cmdId, const nlohmann::json &body)
{
    // ── 소켓 연결 상태 확인 ──
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[NetworkManager] 패킷 전송 실패: 소켓 미연결 상태 ="
                   << m_socket->state();
        return;
    }

    std::string bodyStr = body.dump();
    qDebug() << "[NetworkManager] 패킷 전송 CmdID:" << static_cast<uint16_t>(cmdId)
             << "bodySize:" << bodyStr.size()
             << "body:" << QString::fromStdString(bodyStr);

    PacketHeader header;
    header.signature = 0x4543;
    header.cmdId     = cmdId;
    header.bodySize  = static_cast<uint32_t>(bodyStr.size());

    QByteArray packet;
    packet.append(reinterpret_cast<const char*>(&header), sizeof(PacketHeader));
    packet.append(bodyStr.c_str(), bodyStr.size());

    qint64 written = m_socket->write(packet);
    m_socket->flush();

    qDebug() << "[NetworkManager] 전송 완료, 바이트:" << written;
}

void NetworkManager::handleReadyRead()
{
    m_buffer.append(m_socket->readAll());
    qDebug() << "[NetworkManager] 데이터 수신, 버퍼 크기:" << m_buffer.size();

    while (m_buffer.size() >= static_cast<int>(sizeof(PacketHeader)))
    {
        PacketHeader header;
        std::memcpy(&header, m_buffer.constData(), sizeof(PacketHeader));

        if (header.signature != 0x4543) {
            qWarning() << "[NetworkManager] 잘못된 시그니처:" << header.signature;
            m_buffer.clear();
            return;
        }

        int totalSize = sizeof(PacketHeader) + header.bodySize;
        if (m_buffer.size() < totalSize) {
            qDebug() << "[NetworkManager] 패킷 미완성, 대기중... 버퍼:"
                     << m_buffer.size() << "/" << totalSize;
            break;
        }

        QByteArray body = m_buffer.mid(sizeof(PacketHeader), header.bodySize);
        m_buffer.remove(0, totalSize);

        qDebug() << "[NetworkManager] 패킷 수신 완료 CmdID:"
                 << static_cast<uint16_t>(header.cmdId)
                 << "body:" << body;

        processPacket(header.cmdId, body);
    }
}

void NetworkManager::processPacket(CmdID cmdId, const QByteArray &body)
{
    try {
        nlohmann::json j = nlohmann::json::parse(body.constData());

        if (cmdId == CmdID::RES_LOGIN || cmdId == CmdID::RES_SIGNUP) {
            AuthResDTO dto = j.get<AuthResDTO>();
            qDebug() << "[NetworkManager] 인증 응답 - status:" << dto.status
                     << "message:" << QString::fromStdString(dto.message)
                     << "userName:" << QString::fromStdString(dto.userName);
            emit onAuthResponse(dto.status,
                                QString::fromStdString(dto.message),
                                QString::fromStdString(dto.userName));
        } else {
            qWarning() << "[NetworkManager] 처리되지 않은 CmdID:"
                       << static_cast<uint16_t>(cmdId);
        }

    } catch (const std::exception &e) {
        qWarning() << "[NetworkManager] JSON 파싱 오류:" << e.what()
            << "body:" << body;
    }
}

void NetworkManager::handleConnected()
{
    qDebug() << "[NetworkManager] 서버 연결 성공!";
    emit onConnected();
}

void NetworkManager::handleError(QAbstractSocket::SocketError err)
{
    qWarning() << "[NetworkManager] 소켓 에러:" << err
               << "-" << m_socket->errorString();
}