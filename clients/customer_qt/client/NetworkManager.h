#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include "Global_protocol.h"
#include "BaseDTO.h"
#include "AccountDTO.h"
#include "StoreDTO.h"
#include "ResearchDTO.h"
#include "AddressDTO.h"
#include "OrderDTO.h"
#include "PaymentDTO.h"
#include "StoreDetailDTO.h"
#include "PaymentDTO.h"

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

// 검색 위젯용 Qt 구조체
struct PopularKeywordQt {
    int     rank;
    QString keyword;
};

struct RecentSearchQt {
    int     historyId;
    QString keyword;
    QString searchDate;
};

// 주소 관리용 Qt 구조체
struct AddressItemQt {
    int     addressId   = -1;
    QString address;
    QString detail;
    QString guide;
    QString label       = "기타";
    bool    isDefault   = false;
};

// --- 3페이지(가게 상세)용 Qt 구조체 ---
struct MenuQt {
    int     menuId;
    QString menuName;
    int     basePrice;
    QString description;
    QString imageUrl;
    QString menuCategory;
    bool    isSoldOut;
    bool    isPopular;
};

struct ReviewQt {
    int     reviewId;
    QString userId;
    int     rating;
    QString comment;
    QString ownerReply; // 사장님 답글
    QString createdAt;
    QString orderedMenus; // 주문한 메뉴
};

struct StoreDetailQt {
    int     storeId;
    QString storeName;
    QString storeAddress;
    QString operatingHours;
    QString deliveryFees;
    QString deliveryTimeRange;
    int     minOrderAmount = 0;
    double  rating = 0.0;
    int     reviewCount = 0;
    QString imageUrl;
    
    QList<MenuQt> menus;
    QList<ReviewQt> reviews;
};

// ── 결제 화면 정보용 Qt 구조체 ──
struct CheckoutInfoQt {
    QString customerGrade;   // "일반" or "와우"
    QString cardNumber;      // 카드번호 마스킹
    QString accountNumber;   // 계좌번호
    int     userPoint      = 0;
    int     minOrderAmount = 0;
    int     deliveryFee    = 0;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    void sendMenuReviewRequest(int menuId);
    void connectToServer(const QString &ip, quint16 port);

    // 1등 매장 불러오기
    void sendHeartbeat();

    // ── 인증 관련 ──
    void sendLogin(const LoginReqDTO &dto);
    void sendSignup(const SignupReqDTO &dto);
    void sendIdCheck(const AuthCheckReqDTO &dto);
    void sendPhoneCheck(const PhoneCheckReqDTO &dto);

    // ── 카테고리별 가게 목록 요청 (REQ_STORE_LIST = 2000) ──
    void sendStoreListRequest(int categoryId);

    // ── 매장 검색 요청 (REQ_SEARCH_STORE = 2116) ──
    void sendSearchStore(const QString &keyword);

    // ── 검색 위젯 요청 (REQ_RESEACH_WIDGET = 2108) ──
    void sendSearchWidget(const QString &userId);

    // ── 검색어 추가 요청 (REQ_RESEARCH_ADD = 2112) ──
    void sendSearchAdd(const QString &userId, const QString &keyword);

    // ── 검색어 단건 삭제 요청 (REQ_RESEARCH_DELETE = 2110) ──
    void sendSearchDelete(const QString &userId, int historyId);

    // ── 검색어 전체 삭제 요청 (REQ_RESEARCH_DEL_ALL = 2114) ──
    void sendSearchDeleteAll(const QString &userId);

    // 가게 상세 정보(메뉴, 리뷰 포함) 요청
    void sendStoreDetailRequest(int storeId);

    void sendMenuOptionRequest(int menuId);

    // ── 주소 관리 ──
    void sendAddressSave(const QString &userId, const QString &address,
                         const QString &detail, const QString &guide, const QString &label);
    void sendAddressList(const QString &userId);
    void sendAddressDelete(const QString &userId, int addressId);
    void sendAddressUpdate(const QString &userId, int addressId,
                           const QString &detail, const QString &guide, const QString &label);
    void sendAddressDefault(const QString &userId, int addressId);

    // ── 결제 화면 정보 요청 (REQ_CHECKOUT_INFO = 2026) ──
    void sendCheckoutInfo(const QString &userId, int storeId);

    // ── 주문 생성 요청 (REQ_ORDER_CREATE = 2020) ──
    void sendOrderCreate(const OrderCreateReqDTO &dto);
    void sendOrderHistoryRequest(const QString &userId);
    void sendOrderDetailRequest(const QString &orderId);

signals:
    void onConnected();

    // ── 로그인 응답 ──
    void onLoginResponse(int status, QString message, QString userName, QString address, QString phoneNumber);

    // ── 회원가입 응답 ──
    void onSignupResponse(int status, QString message);

    // ── 아이디/전화번호 중복확인 응답 ──
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);

    // ── 메인 홈 데이터 수신 (RES_CATEGORY) ──
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

    // ── 카테고리별 가게 목록 수신 (RES_STORE_LIST = 2001) ──
    void onStoreListReceived(QList<TopStoreInfoQt> stores);

    // ── 1등 가게 새로고침(홈화면) (RES_HEARTBEAT = 1001) ──
    void onHeartbeatReceived(QList<TopStoreInfoQt> topStores);

    // ── 매장 검색 결과 수신 (RES_SEARCH_STORE = 2117) ──
    void onSearchResultReceived(QList<TopStoreInfoQt> stores);

    // ── 검색 위젯 데이터 수신 (RES_RESEACH_WIDGET = 2109) ──
    void onSearchWidgetReceived(QList<PopularKeywordQt> popular,
                                QList<RecentSearchQt> recent);

    // 가게 상세 정보 수신 완료 신호
    void onStoreDetailReceived(StoreDetailQt detail);

    void onMenuOptionsReceived(int menuId, QList<OptionGroup> groups);

    // ── 주소 관리 응답 ──
    void onAddressSaveReceived(int status, int addressId);
    void onAddressListReceived(QList<AddressItemQt> addresses);
    void onAddressDeleteReceived(int status);
    void onAddressUpdateReceived(int status);
    void onAddressDefaultReceived(int status);

    // ── 결제 화면 정보 응답 ──
    // void onCheckoutInfoReceived(CheckoutInfoQt info);

    // ── 주문 생성 응답 ──
    void onOrderCreateReceived(int status, QString message, QString orderId);
    void onOrderStateChanged(int state, const QString &orderId);
    void onMenuReviewsReceived(int menuId, QList<ReviewDTO> reviews);
    void onDeliveryCompleteReceived(const QString &orderId); // 배달 완료 신호 (4011번) 수신 시 발생할 시그널
    void onOrderHistoryReceived(const ResOrderHistoryDTO &resDto);
    void onOrderDetailReceived(const ResOrderDetailDTO &resDto);

    void onCheckoutInfoReceived(int status, QString customerGrade,
                                int deliveryFee, int minOrderAmount,
                                QString pickupTime, QString cardNumber, QString accountNumber);


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
