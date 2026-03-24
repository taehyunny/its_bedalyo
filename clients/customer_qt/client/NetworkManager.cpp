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

// ── 검색 위젯 요청 (REQ_RESEARCH_WIDGET = 2108) ──
void NetworkManager::sendSearchWidget(const QString &userId)
{
    qDebug() << "[NetworkManager] 검색 위젯 요청 userId:" << userId;
    ReqResearchWidgetDTO dto;
    dto.userId = userId.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_RESEARCH_WIDGET, j);
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
    sendPacket(CmdID::REQ_STORE_DETAIL, j);
}

// ── 주소 저장 요청 (REQ_ADDRESS_SAVE = 2070) ──
void NetworkManager::sendAddressSave(const QString &userId, const QString &address,
                                      const QString &detail, const QString &guide, const QString &label)
{
    qDebug() << "[NetworkManager] 주소 저장 요청:" << address;
    ReqAddressSaveDTO dto;
    dto.userId  = userId.toStdString();
    dto.address = address.toStdString();
    dto.detail  = detail.toStdString();
    dto.guide   = guide.toStdString();
    dto.label   = label.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ADDRESS_SAVE, j);
}

// ── 주소 목록 요청 (REQ_ADDRESS_LIST = 2072) ──
void NetworkManager::sendAddressList(const QString &userId)
{
    qDebug() << "[NetworkManager] 주소 목록 요청 userId:" << userId;
    ReqAddressListDTO dto;
    dto.userId = userId.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ADDRESS_LIST, j);
}

// ── 주소 삭제 요청 (REQ_ADDRESS_DELETE = 2074) ──
void NetworkManager::sendAddressDelete(const QString &userId, int addressId)
{
    qDebug() << "[NetworkManager] 주소 삭제 요청 addressId:" << addressId;
    ReqAddressDeleteDTO dto;
    dto.userId    = userId.toStdString();
    dto.addressId = addressId;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ADDRESS_DELETE, j);
}

// ── 주소 수정 요청 (REQ_ADDRESS_UPDATE = 2076) ──
void NetworkManager::sendAddressUpdate(const QString &userId, int addressId,
                                        const QString &detail, const QString &guide, const QString &label)
{
    qDebug() << "[NetworkManager] 주소 수정 요청 addressId:" << addressId;
    ReqAddressUpdateDTO dto;
    dto.userId    = userId.toStdString();
    dto.addressId = addressId;
    dto.detail    = detail.toStdString();
    dto.guide     = guide.toStdString();
    dto.label     = label.toStdString();
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ADDRESS_UPDATE, j);
}

// ── 기본 주소 변경 요청 (REQ_ADDRESS_DEFAULT = 2078) ──
void NetworkManager::sendAddressDefault(const QString &userId, int addressId)
{
    qDebug() << "[NetworkManager] 기본 주소 변경 요청 addressId:" << addressId;
    ReqAddressDefaultDTO dto;
    dto.userId    = userId.toStdString();
    dto.addressId = addressId;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ADDRESS_DEFAULT, j);
}

// ── 결제 화면 정보 요청 (REQ_CHECKOUT_INFO = 2026) ──
void NetworkManager::sendCheckoutInfo(const QString &userId, int storeId)
{
    qDebug() << "[NetworkManager] 결제 화면 정보 요청 storeId:" << storeId;
    ReqCheckoutInfoDTO dto;
    dto.userId  = userId.toStdString();
    dto.storeId = storeId;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_CHECKOUT_INFO, j);
}

// ── 주문 생성 요청 (REQ_ORDER_CREATE = 2020) ──
void NetworkManager::sendOrderCreate(const OrderCreateReqDTO &dto)
{
    qDebug() << "[NetworkManager] 주문 생성 요청 storeId:" << dto.storeId;
    nlohmann::json j = dto;
    sendPacket(CmdID::REQ_ORDER_CREATE, j);
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
            qDebug() << "[Login] phoneNumber:" << QString::fromStdString(dto.phoneNumber);
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

        // ── 검색 위젯 데이터 수신 (RES_RESEARCH_WIDGET = 2109) ──
        } else if (cmdId == CmdID::RES_RESEARCH_WIDGET) {
            qDebug() << "[DEBUG] RES_RESEARCH_WIDGET raw:" << QString::fromUtf8(body);

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
                item.historyId  = r.historyId;
                item.keyword    = QString::fromStdString(r.keyword);
                item.searchDate = QString::fromStdString(r.searchDate);
                recent.append(item);
            }

            emit onSearchWidgetReceived(popular, recent);

        // ── 검색어 단건 삭제 응답 (RES_RESEARCH_DELETE = 2111) ──
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
        } else if (cmdId == CmdID::RES_RESEARCH_DEL_ALL) {
            ResResearchDelAllDTO dto = j.get<ResResearchDelAllDTO>();
            if (dto.status != 200)
                qWarning() << "[NetworkManager] 검색어 전체 삭제 실패 status:" << dto.status;

        // ── 가게 상세 정보(3페이지) 데이터 수신 ──
        } else if (cmdId == CmdID::RES_STORE_DETAIL) {
            ResStoreDetailDTO dto = j.get<ResStoreDetailDTO>();

            if (dto.status != 200) {
                qWarning() << "매장 정보를 불러오지 못했습니다. status:" << dto.status;
                return;
            }

            StoreDetailQt detail;
            // [수정] StoreDetailDTO.h 멤버명은 camelCase → 일치시킴
            detail.storeId           = dto.storeData.storeId;
            detail.storeName         = QString::fromStdString(dto.storeData.storeName);
            detail.storeAddress      = QString::fromStdString(dto.storeData.storeAddress);
            detail.operatingHours    = QString::fromStdString(dto.storeData.operatingHours);
            detail.deliveryFees      = QString::fromStdString(dto.storeData.deliveryFees);
            detail.deliveryTimeRange = QString::fromStdString(dto.storeData.deliveryTimeRange);
            detail.minOrderAmount    = dto.storeData.minOrderAmount;
            detail.rating            = dto.storeData.rating;
            detail.reviewCount       = dto.storeData.reviewCount;
            detail.imageUrl          = QString::fromStdString(dto.storeData.imageUrl);

            for (const auto &m : dto.menuList) {
                MenuQt menu;
                menu.menuId       = m.menuId;
                menu.menuName     = QString::fromStdString(m.menuName);
                menu.basePrice    = m.basePrice;
                menu.description  = QString::fromStdString(m.description);
                menu.imageUrl     = QString::fromStdString(m.imageUrl);
                menu.menuCategory = QString::fromStdString(m.menuCategory);
                menu.isSoldOut    = m.isSoldOut;
                menu.isPopular    = m.isPopular;
                detail.menus.append(menu);
            }

            for (const auto &r : dto.reviewList) {
                ReviewQt review;
                review.reviewId  = r.reviewId;
                review.userId    = QString::fromStdString(r.userId);
                review.rating    = r.rating;
                review.comment   = QString::fromStdString(r.content);
                review.createdAt = QString::fromStdString(r.createdAt);
                detail.reviews.append(review);
            }

            emit onStoreDetailReceived(detail);

        // ── 주소 저장 응답 (RES_ADDRESS_SAVE = 2071) ──
        } else if (cmdId == CmdID::RES_ADDRESS_SAVE) {
            ResAddressSaveDTO dto = j.get<ResAddressSaveDTO>();
            emit onAddressSaveReceived(dto.status, dto.addressId);

        // ── 주소 목록 응답 (RES_ADDRESS_LIST = 2073) ──
        } else if (cmdId == CmdID::RES_ADDRESS_LIST) {
            ResAddressListDTO dto = j.get<ResAddressListDTO>();
            QList<AddressItemQt> list;
            for (const auto &a : dto.addresses) {
                AddressItemQt item;
                item.addressId = a.addressId;
                item.address   = QString::fromStdString(a.address);
                item.detail    = QString::fromStdString(a.detail);
                item.guide     = QString::fromStdString(a.guide);
                item.label     = QString::fromStdString(a.label);
                item.isDefault = a.isDefault;
                list.append(item);
            }
            emit onAddressListReceived(list);

        // ── 주소 삭제 응답 (RES_ADDRESS_DELETE = 2075) ──
        } else if (cmdId == CmdID::RES_ADDRESS_DELETE) {
            ResAddressDeleteDTO dto = j.get<ResAddressDeleteDTO>();
            emit onAddressDeleteReceived(dto.status);

        // ── 주소 수정 응답 (RES_ADDRESS_UPDATE = 2077) ──
        } else if (cmdId == CmdID::RES_ADDRESS_UPDATE) {
            ResAddressUpdateDTO dto = j.get<ResAddressUpdateDTO>();
            emit onAddressUpdateReceived(dto.status);

        // ── 기본 주소 변경 응답 (RES_ADDRESS_DEFAULT = 2079) ──
        } else if (cmdId == CmdID::RES_ADDRESS_DEFAULT) {
            ResAddressDefaultDTO dto = j.get<ResAddressDefaultDTO>();
            emit onAddressDefaultReceived(dto.status);

        // ── 결제 화면 정보 응답 (RES_CHECKOUT_INFO = 2027) ──
        // [수정] 중복 핸들러 제거 → 4-param 시그널만 사용
        } else if (cmdId == CmdID::RES_CHECKOUT_INFO) {
            ResCheckoutInfoDTO dto = j.get<ResCheckoutInfoDTO>();
            emit onCheckoutInfoReceived(dto.status,
                                        QString::fromStdString(dto.customerGrade),
                                        dto.deliveryFee,
                                        dto.minOrderAmount);

        // ── 주문 생성 응답 (RES_ORDER_CREATE = 2021) ──
        } else if (cmdId == CmdID::RES_ORDER_CREATE) {
            OrderCreateResDTO dto = j.get<OrderCreateResDTO>();
            emit onOrderCreateReceived(dto.status,
                                       QString::fromStdString(dto.message),
                                       QString::fromStdString(dto.orderId));

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
