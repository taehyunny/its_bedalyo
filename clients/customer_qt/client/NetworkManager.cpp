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

// ── 인증 관련 ──

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

// ── 카테고리별 가게 목록 요청 (REQ_STORE_LIST = 2000) ──
void NetworkManager::sendStoreListRequest(int categoryId)
{
    qDebug() << "[NetworkManager] 가게 목록 요청 categoryId:" << categoryId;
    nlohmann::json j;
    j["categoryId"] = categoryId;
    sendPacket(CmdID::REQ_STORE_LIST, j);
}

// ── 매장 검색 요청 (REQ_SEARCH_STORE = 2116) ──
void NetworkManager::sendSearchStore(const QString &keyword)
{
    qDebug() << "[NetworkManager] 매장 검색 요청 keyword:" << keyword;
    ReqSearchStoreDTO dto;
    dto.keyword = keyword.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_SEARCH_STORE, j);
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

        // ── 로그인 응답 ──
        if (cmdId == CmdID::RES_LOGIN) {
            AuthResDTO dto = j.get<AuthResDTO>();
            qDebug() << "[NetworkManager] 로그인 응답 status:" << dto.status
                     << "userName:" << QString::fromStdString(dto.userName)
                     << "address:"  << QString::fromStdString(dto.address);
            emit onLoginResponse(dto.status,
                                 QString::fromStdString(dto.message),
                                 QString::fromStdString(dto.userName),
                                 QString::fromStdString(dto.address),
                                 QString::fromStdString(dto.phoneNumber));

        // ── 회원가입 응답 ──
        } else if (cmdId == CmdID::RES_SIGNUP) {
            AuthResDTO dto = j.get<AuthResDTO>();
            qDebug() << "[NetworkManager] 회원가입 응답 status:" << dto.status;
            emit onSignupResponse(dto.status,
                                  QString::fromStdString(dto.message));

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

        // ── 메인 홈 데이터 수신 (RES_CATEGORY) ──
        } else if (cmdId == CmdID::RES_CATEGORY) {
            MainHomeResDTO dto = j.get<MainHomeResDTO>();
            qDebug() << "[NetworkManager] 메인홈 수신 - 카테고리:"
                     << dto.categories.size() << "가게:" << dto.topStores.size();

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
                item.storeId           = s.storeId;
                item.storeName         = QString::fromStdString(s.storeName);
                item.category          = QString::fromStdString(s.category);
                item.iconPath          = QString::fromStdString(s.iconPath);
                item.deliveryTimeRange = QString::fromStdString(s.deliveryTimeRange);
                item.rating            = s.rating;
                item.reviewCount       = s.reviewCount;
                item.minOrderAmount    = s.minOrderAmount;
                item.deliveryFee       = s.deliveryFee;
                topStores.append(item);
            }

            emit onMainHomeReceived(categories, topStores);

        // ── 카테고리별 가게 목록 수신 (RES_STORE_LIST = 2001) ──
        // 서버 구조: { "status": 200, "message": "...", "stores": [...] }
        // → StoreListResDTO로 파싱 후 stores 배열 사용
        } else if (cmdId == CmdID::RES_STORE_LIST) {
            StoreListResDTO dto = j.get<StoreListResDTO>();
            qDebug() << "[NetworkManager] 가게 목록 수신 status:" << dto.status
                     << "count:" << dto.stores.size();

            QList<TopStoreInfoQt> stores;
            for (const auto &s : dto.stores) {
                TopStoreInfoQt item;
                item.storeId           = s.storeId;
                item.storeName         = QString::fromStdString(s.storeName);
                item.category          = QString::fromStdString(s.category);
                item.iconPath          = QString::fromStdString(s.iconPath);
                item.deliveryTimeRange = QString::fromStdString(s.deliveryTimeRange);
                item.rating            = s.rating;
                item.reviewCount       = s.reviewCount;
                item.minOrderAmount    = s.minOrderAmount;
                item.deliveryFee       = s.deliveryFee;
                stores.append(item);
            }

            emit onStoreListReceived(stores);

        // ── 매장 검색 결과 수신 (RES_SEARCH_STORE = 2117) ──
        } else if (cmdId == CmdID::RES_SEARCH_STORE) {
            ResSearchStoreDTO dto = j.get<ResSearchStoreDTO>();
            qDebug() << "[NetworkManager] 매장 검색 결과 status:" << dto.status
                     << "count:" << dto.storeList.size();

            QList<TopStoreInfoQt> stores;
            for (const auto &s : dto.storeList) {
                TopStoreInfoQt item;
                item.storeId           = s.storeId;
                item.storeName         = QString::fromStdString(s.storeName);
                item.category          = QString::fromStdString(s.category);
                item.iconPath          = QString::fromStdString(s.iconPath);
                item.deliveryTimeRange = QString::fromStdString(s.deliveryTimeRange);
                item.rating            = s.rating;
                item.reviewCount       = s.reviewCount;
                item.minOrderAmount    = s.minOrderAmount;
                item.deliveryFee       = s.deliveryFee;
                stores.append(item);
            }
            emit onSearchResultReceived(stores);

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
