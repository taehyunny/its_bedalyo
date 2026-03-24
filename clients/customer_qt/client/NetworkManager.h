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

// Qt용 구조체 정의 (생략 없이 유지)
struct CategoryInfoQt { int id; QString name; QString iconPath; };
struct TopStoreInfoQt {
    int storeId; QString storeName; QString category; QString iconPath;
    QString deliveryTimeRange; double rating = 0.0; int reviewCount = 0;
    int minOrderAmount = 0; int deliveryFee = 0;
};
struct PopularKeywordQt { int rank; QString keyword; };
struct RecentSearchQt { int historyId; QString keyword; QString searchDate; };
struct AddressItemQt {
    int addressId = -1; QString address; QString detail; QString guide;
    QString label = "기타"; bool isDefault = false;
};
struct MenuQt {
    int menuId; QString menuName; int basePrice; QString description;
    QString imageUrl; QString menuCategory; bool isSoldOut; bool isPopular;
};


struct ReviewQt {
    int     reviewId;
    QString userId;
    int     rating;
    QString comment;
    QString createdAt;
    QString ownerReply;   // 사장님 답글
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
// (NetworkManager 내부에서만 사용. 외부에는 개별 파라미터로 시그널 전달)
struct CheckoutInfoQt {
    QString customerGrade;
    QString cardNumber;
    QString accountNumber;
    int     userPoint      = 0;
    int     minOrderAmount = 0;
    int     deliveryFee    = 0;

struct ReviewQt { int reviewId; QString userId; int rating; QString comment; QString createdAt; };
struct StoreDetailQt {
    int storeId; QString storeName; QString storeAddress; QString operatingHours;
    QString deliveryFees; QString deliveryTimeRange; int minOrderAmount = 0;
    double rating = 0.0; int reviewCount = 0; QString imageUrl;
    QList<MenuQt> menus; QList<ReviewQt> reviews;

};

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    void connectToServer(const QString &ip, quint16 port);

    // 요청 함수들
    void sendLogin(const LoginReqDTO &dto);
    void sendSignup(const SignupReqDTO &dto);
    void sendIdCheck(const AuthCheckReqDTO &dto);
    void sendPhoneCheck(const PhoneCheckReqDTO &dto);
    void sendStoreListRequest(int categoryId);
    void sendSearchStore(const QString &keyword);
    void sendSearchWidget(const QString &userId);
    void sendSearchAdd(const QString &userId, const QString &keyword);
    void sendSearchDelete(const QString &userId, int historyId);
    void sendSearchDeleteAll(const QString &userId);
    void sendStoreDetailRequest(int storeId);
    void sendMenuOptionRequest(int menuId); // 메뉴 옵션 요청
    void sendAddressSave(const QString &userId, const QString &address, const QString &detail, const QString &guide, const QString &label);
    void sendAddressList(const QString &userId);
    void sendAddressDelete(const QString &userId, int addressId);
    void sendAddressUpdate(const QString &userId, int addressId, const QString &detail, const QString &guide, const QString &label);
    void sendAddressDefault(const QString &userId, int addressId);
    void sendCheckoutInfo(const QString &userId, int storeId);
    void sendOrderCreate(const OrderCreateReqDTO &dto);

    // 범용 전송 함수 (public으로 변경)
    void sendPacket(CmdID cmdId, const nlohmann::json &body);

signals:
    void onConnected();
    void onLoginResponse(int status, QString message, QString userName, QString address, QString phoneNumber);
    void onSignupResponse(int status, QString message);
    void onIdCheckResponse(int status, QString message, bool isAvailable);
    void onPhoneCheckResponse(int status, QString message, bool isAvailable);
    void onMainHomeReceived(QList<CategoryInfoQt> categories, QList<TopStoreInfoQt> topStores);
    void onStoreListReceived(QList<TopStoreInfoQt> stores);
    void onSearchResultReceived(QList<TopStoreInfoQt> stores);
    void onSearchWidgetReceived(QList<PopularKeywordQt> popular, QList<RecentSearchQt> recent);
    void onStoreDetailReceived(StoreDetailQt detail);
    void onAddressSaveReceived(int status, int addressId);
    void onAddressListReceived(QList<AddressItemQt> addresses);
    void onAddressDeleteReceived(int status);
    void onAddressUpdateReceived(int status);
    void onAddressDefaultReceived(int status);
    void onCheckoutInfoReceived(int status, QString customerGrade, int deliveryFee, int minOrderAmount);
    void onOrderCreateReceived(int status, QString message, QString orderId);

    // 메뉴 옵션 수신 시그널 (중복 제거됨)
    void onMenuOptionsReceived(int menuId, QList<OptionGroup> groups);

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *m_socket;
    QByteArray  m_buffer;
    void processPacket(CmdID cmdId, const QByteArray &body);
};