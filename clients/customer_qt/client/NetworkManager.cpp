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

// ── 검색 위젯 요청 (REQ_RESEACH_WIDGET = 2108) ──
void NetworkManager::sendSearchWidget(const QString &userId)
{
    qDebug() << "[NetworkManager] 검색 위젯 요청 userId:" << userId;
    ReqResearchWidgetDTO dto;
    dto.userId = userId.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_RESEACH_WIDGET, j);
}

// ── 검색어 추가 요청 (REQ_RESEARCH_ADD = 2112) ──
void NetworkManager::sendSearchAdd(const QString &userId, const QString &keyword)
{
    qDebug() << "[NetworkManager] 검색어 추가 요청 keyword:" << keyword;
    ReqResearchAddDTO dto;
    dto.userId  = userId.toStdString();
    dto.keyword = keyword.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_RESEARCH_ADD, j);
}

// ── 검색어 단건 삭제 요청 (REQ_RESEARCH_DELETE = 2110) ──
void NetworkManager::sendSearchDelete(const QString &userId, int historyId)
{
    qDebug() << "[NetworkManager] 검색어 단건 삭제 historyId:" << historyId;
    ReqResearchDeleteDTO dto;
    dto.userId    = userId.toStdString();
    dto.historyId = historyId;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_RESEARCH_DELETE, j);
}

// ── 검색어 전체 삭제 요청 (REQ_RESEARCH_DEL_ALL = 2114) ──
void NetworkManager::sendSearchDeleteAll(const QString &userId)
{
    qDebug() << "[NetworkManager] 검색어 전체 삭제 userId:" << userId;
    ReqResearchDelAllDTO dto;
    dto.userId = userId.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_RESEARCH_DEL_ALL, j);
}

// ── 가게 상세 정보(메뉴+리뷰) 요청 ──
void NetworkManager::sendStoreDetailRequest(int storeId)
{
    qDebug() << "[NetworkManager] 가게 상세 정보 요청 storeId:" << storeId;
    nlohmann::json j;
    j["storeId"] = storeId;
    // CmdID::REQ_STORE_DETAIL 부분은 Global_protocol.h에 맞춰 이름을 변경해야 할 수 있습니다
    sendPacket(CmdID::REQ_STORE_DETAIL, j); 
}

// ============================================================
// TopStoreInfo(C++ DTO) → TopStoreInfoQt(Qt 타입) 변환 헬퍼
// ============================================================
static TopStoreInfoQt toQt(const TopStoreInfo &s)
{
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
    return item;
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
            emit onLoginResponse(dto.status,
                                 QString::fromStdString(dto.message),
                                 QString::fromStdString(dto.userName),
                                 QString::fromStdString(dto.address),
                                 QString::fromStdString(dto.phoneNumber));

        // ── 회원가입 응답 ──
        } else if (cmdId == CmdID::RES_SIGNUP) {
            AuthResDTO dto = j.get<AuthResDTO>();
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

            QList<CategoryInfoQt> categories;
            for (const auto &c : dto.categories) {
                CategoryInfoQt item;
                item.id       = c.id;
                item.name     = QString::fromStdString(c.name);
                item.iconPath = QString::fromStdString(c.iconPath);
                categories.append(item);
            }

            QList<TopStoreInfoQt> topStores;
            for (const auto &s : dto.topStores)
                topStores.append(toQt(s));

            emit onMainHomeReceived(categories, topStores);

        // ── 카테고리별 가게 목록 수신 (RES_STORE_LIST = 2001) ──
        } else if (cmdId == CmdID::RES_STORE_LIST) {
            StoreListResDTO dto = j.get<StoreListResDTO>();

            QList<TopStoreInfoQt> stores;
            for (const auto &s : dto.stores)
                stores.append(toQt(s));

            emit onStoreListReceived(stores);

        // ── 매장 검색 결과 수신 (RES_SEARCH_STORE = 2117) ──
        } else if (cmdId == CmdID::RES_SEARCH_STORE) {
            ResSearchStoreDTO dto = j.get<ResSearchStoreDTO>();

            QList<TopStoreInfoQt> stores;
            for (const auto &s : dto.storeList)
                stores.append(toQt(s));

            emit onSearchResultReceived(stores);

        // ── 검색 위젯 데이터 수신 (RES_RESEACH_WIDGET = 2109) ──
        } else if (cmdId == CmdID::RES_RESEACH_WIDGET) {
            // ── 디버그: 서버 응답 원문 출력 ──
            qDebug() << "[DEBUG] RES_RESEACH_WIDGET raw:" << QString::fromUtf8(body);

            ResResearchWidgetDTO dto = j.get<ResResearchWidgetDTO>();

            QList<PopularKeywordQt> popular;
            for (const auto &p : dto.popularKeywords) {
                PopularKeywordQt item;
                item.rank    = p.rank;
                item.keyword = QString::fromStdString(p.keyword);
                popular.append(item);
            }

            QList<RecentSearchQt> recent;
            for (const auto &r : dto.recentSearches) {
                RecentSearchQt item;
                item.historyId = r.historyId;
                item.keyword   = QString::fromStdString(r.keyword);
                item.searchDate = QString::fromStdString(r.searchDate);
                recent.append(item);
            }

            emit onSearchWidgetReceived(popular, recent);

        // ── 검색어 단건 삭제 응답 (RES_RESEARCH_DELETE = 2111) ──
        // UI는 낙관적 업데이트로 이미 처리됨 → 실패 시에만 경고 로그
        } else if (cmdId == CmdID::RES_RESEARCH_DELETE) {
            ResResearchDeleteDTO dto = j.get<ResResearchDeleteDTO>();
            if (dto.status != 200)
                qWarning() << "[NetworkManager] 검색어 단건 삭제 실패 status:" << dto.status;

        // ── 검색어 추가 응답 (RES_RESEARCH_ADD = 2113) ──
        } else if (cmdId == CmdID::RES_RESEARCH_ADD) {
            ResResearchAddDTO dto = j.get<ResResearchAddDTO>();
            if (dto.status != 200)
                qWarning() << "[NetworkManager] 검색어 추가 실패 status:" << dto.status;

        // ── 검색어 전체 삭제 응답 (RES_RESEARCH_DEL_ALL = 2115) ──
        // UI는 낙관적 업데이트로 이미 처리됨 → 실패 시에만 경고 로그
        } else if (cmdId == CmdID::RES_RESEARCH_DEL_ALL) {
            ResResearchDelAllDTO dto = j.get<ResResearchDelAllDTO>();
            if (dto.status != 200)
                qWarning() << "[NetworkManager] 검색어 전체 삭제 실패 status:" << dto.status;

        // ── 가게 상세 정보(3페이지) 데이터 수신 ──
        } else if (cmdId == CmdID::RES_STORE_DETAIL) {
            // (서버의 ResStoreDetailDTO를 파싱합니다)
            ResStoreDetailDTO dto = j.get<ResStoreDetailDTO>();
            
            if (dto.status != 200) {
                qWarning() << "매장 정보를 불러오지 못했습니다. status:" << dto.status;
                return;
            }

            StoreDetailQt detail;
            detail.storeId           = dto.storeData.store_id;
            detail.storeName         = QString::fromStdString(dto.storeData.store_name);
            detail.storeAddress      = QString::fromStdString(dto.storeData.store_address);
            detail.operatingHours    = QString::fromStdString(dto.storeData.operating_hours);
            detail.deliveryFees      = QString::fromStdString(dto.storeData.delivery_fees);
            detail.deliveryTimeRange = QString::fromStdString(dto.storeData.delivery_time_range);
            detail.minOrderAmount    = dto.storeData.min_order_amount;
            detail.rating            = dto.storeData.rating;
            detail.reviewCount       = dto.storeData.review_count;
            detail.imageUrl          = QString::fromStdString(dto.storeData.image_url);

            // 메뉴 리스트 파싱
            for (const auto &m : dto.menuList) {
                MenuQt menu;
                menu.menuId       = m.menu_id;
                menu.menuName     = QString::fromStdString(m.menu_name);
                menu.basePrice    = m.base_price;
                menu.description  = QString::fromStdString(m.description);
                menu.imageUrl     = QString::fromStdString(m.image_url);
                menu.menuCategory = QString::fromStdString(m.menu_category);
                menu.isSoldOut    = m.is_sold_out;
                menu.isPopular    = m.is_popular;
                detail.menus.append(menu);
            }

            // 리뷰 리스트 파싱
            for (const auto &r : dto.reviewList) {
                ReviewQt review;
                review.reviewId  = r.review_id;
                review.userId    = QString::fromStdString(r.user_id);
                review.rating    = r.rating;
                review.comment   = QString::fromStdString(r.comment);
                review.createdAt = QString::fromStdString(r.created_at);
                detail.reviews.append(review);
            }

            emit onStoreDetailReceived(detail);        
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
