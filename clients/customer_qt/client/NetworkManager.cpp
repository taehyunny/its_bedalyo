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
    qDebug() << "[NetworkManager] 회원가입 요청 -" << QString::fromStdString(dto.userId);
    nlohmann::json j;
    to_json(j, dto);
    sendPacket(CmdID::REQ_SIGNUP, j);
}

void NetworkManager::sendIdCheck(const AuthCheckReqDTO &dto)
{
    qDebug() << "[NetworkManager] 아이디 중복확인 -" << QString::fromStdString(dto.userId);
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_AUTH_CHECK, j);
}

void NetworkManager::sendPhoneCheck(const PhoneCheckReqDTO &dto)
{
    qDebug() << "[NetworkManager] 전화번호 중복확인 -"
             << QString::fromStdString(dto.phoneNumber) << "role:" << dto.role;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_PHONE_CHECK, j);
}

void NetworkManager::sendPacket(CmdID cmdId, const nlohmann::json &body)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[NetworkManager] 패킷 전송 실패: 소켓 미연결";
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

    qDebug() << "[NetworkManager] 전송 CmdID:" << static_cast<uint16_t>(cmdId)
             << "size:" << jsonBytes.size();

    m_socket->write(packet);
    m_socket->flush();
}

void NetworkManager::handleReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (m_buffer.size() >= static_cast<int>(sizeof(PacketHeader)))
    {
        QDataStream headerStream(m_buffer);
        headerStream.setByteOrder(QDataStream::BigEndian);

        quint16 signature, cmdIdRaw;
        quint32 bodySize;
        headerStream >> signature >> cmdIdRaw >> bodySize;

        if (signature != 0x4543) {
            qWarning() << "[NetworkManager] 잘못된 시그니처";
            m_buffer.clear();
            return;
        }

        int totalSize = sizeof(PacketHeader) + bodySize;
        if (m_buffer.size() < totalSize) break;

        QByteArray body = m_buffer.mid(sizeof(PacketHeader), bodySize);
        m_buffer.remove(0, totalSize);

        qDebug() << "[NetworkManager] 수신 CmdID:" << cmdIdRaw;
        processPacket(static_cast<CmdID>(cmdIdRaw), body);
    }
}

void NetworkManager::processPacket(CmdID cmdId, const QByteArray &body)
{
    try {
        nlohmann::json j = nlohmann::json::parse(body.constData());

        // ── 로그인 / 회원가입 응답 ──
        if (cmdId == CmdID::RES_LOGIN || cmdId == CmdID::RES_SIGNUP) {
            AuthResDTO dto = j.get<AuthResDTO>();
            qDebug() << "[NetworkManager] 인증응답 status:" << dto.status
                     << "userName:" << QString::fromStdString(dto.userName);
            emit onAuthResponse(dto.status,
                                QString::fromStdString(dto.message),
                                QString::fromStdString(dto.userName));

        // ── 아이디 중복확인 응답 ──
        } else if (cmdId == CmdID::RES_AUTH_CHECK) {
            AuthCheckResDTO dto = j.get<AuthCheckResDTO>();
            emit onIdCheckResponse(dto.status,
                                   QString::fromStdString(dto.message),
                                   dto.isAvailable);

        // ── 전화번호 중복확인 응답 ──
        } else if (cmdId == CmdID::RES_PHONE_CHECK) {
            PhoneCheckResDTO dto = j.get<PhoneCheckResDTO>();
            emit onPhoneCheckResponse(dto.status,
                                      QString::fromStdString(dto.message),
                                      dto.isAvailable);

        // ── 메인 화면 데이터 (연결 시 서버가 자동 push) ──
        } else if (cmdId == CmdID::RES_CATEGORY) {
            MainHomeResDTO dto = j.get<MainHomeResDTO>();
            qDebug() << "[NetworkManager] 메인홈 수신 - 카테고리:"
                     << dto.categories.size() << "가게:" << dto.topStores.size();

            // C++ DTO → Qt 구조체 변환
            QList<CategoryInfoQt> categories;
            for (const auto &c : dto.categories) {
                CategoryInfoQt item;
                item.id       = c.id;
                item.name     = QString::fromStdString(c.name);
                item.iconPath = QString::fromStdString(c.iconPath);
                categories.append(item);
            }

            QList<TopStoreInfoQt> topStores;
            for (const auto &s : dto.topStores) {
                TopStoreInfoQt item;
                item.storeId          = s.storeId;
                item.storeName        = QString::fromStdString(s.storeName);
                item.category         = QString::fromStdString(s.category);
                item.iconPath         = QString::fromStdString(s.iconPath);
                item.deliveryTimeRange = QString::fromStdString(s.deliveryTimeRange);
                item.rating           = s.rating;
                item.reviewCount      = s.reviewCount;
                item.minOrderAmount   = s.minOrderAmount;
                item.deliveryFee      = s.deliveryFee;
                topStores.append(item);
            }

            emit onMainHomeReceived(categories, topStores);

        } else {
            qWarning() << "[NetworkManager] 처리되지 않은 CmdID:"
                       << static_cast<uint16_t>(cmdId);
        }

    } catch (const std::exception &e) {
        qWarning() << "[NetworkManager] JSON 파싱 오류:" << e.what();
    }
}

void NetworkManager::handleConnected()
{
    qDebug() << "[NetworkManager] 서버 연결 성공!";
    emit onConnected();
}

void NetworkManager::handleError(QAbstractSocket::SocketError err)
{
    qWarning() << "[NetworkManager] 소켓 에러:" << err << "-" << m_socket->errorString();
}
