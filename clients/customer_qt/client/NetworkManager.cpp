#include "NetworkManager.h"
#include "json.hpp"
#include <QDebug>
#include <QDataStream>
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
    connect(m_socket, &QTcpSocket::disconnected, this, []() {
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

// ============================================================
// 패킷 조립 및 전송
// 서버: memcpy로 raw 읽기 → ntohs/ntohl 변환 → 비교
// 클라이언트: Little Endian(호스트 바이트 오더)으로 전송
// ============================================================
void NetworkManager::sendPacket(CmdID cmdId, const nlohmann::json &body)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[NetworkManager] 패킷 전송 실패: 소켓 미연결 상태 ="
                   << m_socket->state();
        return;
    }

    QByteArray jsonBytes = QByteArray::fromStdString(body.dump());

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << quint16(0x4543);
    stream << quint16(static_cast<uint16_t>(cmdId));
    stream << quint32(jsonBytes.size());
    packet.append(jsonBytes);

    qDebug() << "[NetworkManager] 패킷 전송 CmdID:" << static_cast<uint16_t>(cmdId)
             << "bodySize:" << jsonBytes.size()
             << "body:" << jsonBytes;

    qint64 written = m_socket->write(packet);
    m_socket->flush();
    qDebug() << "[NetworkManager] 전송 완료, 바이트:" << written;
}

// ============================================================
// 데이터 수신 처리
// 서버도 동일하게 Little Endian으로 전송하므로 수신도 동일하게 파싱
// ============================================================
void NetworkManager::handleReadyRead()
{
    m_buffer.append(m_socket->readAll());
    qDebug() << "[NetworkManager] 데이터 수신, 버퍼 크기:" << m_buffer.size();

    while (m_buffer.size() >= static_cast<int>(sizeof(PacketHeader)))
    {
        QDataStream headerStream(m_buffer);
        headerStream.setByteOrder(QDataStream::BigEndian);

        quint16 signature, cmdIdRaw;
        quint32 bodySize;
        headerStream >> signature >> cmdIdRaw >> bodySize;

        if (signature != 0x4543) {
            qWarning() << "[NetworkManager] 잘못된 시그니처:" << Qt::hex << signature;
            m_buffer.clear();
            return;
        }

        int totalSize = sizeof(PacketHeader) + bodySize;
        if (m_buffer.size() < totalSize) {
            qDebug() << "[NetworkManager] 패킷 미완성, 대기중..."
                     << m_buffer.size() << "/" << totalSize;
            break;
        }

        QByteArray body = m_buffer.mid(sizeof(PacketHeader), bodySize);
        m_buffer.remove(0, totalSize);

        qDebug() << "[NetworkManager] 패킷 수신 완료 CmdID:" << cmdIdRaw
                 << "body:" << body;

        processPacket(static_cast<CmdID>(cmdIdRaw), body);
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