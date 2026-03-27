#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include "NetworkManager.h"
#include "cartsession.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~CartWidget();

public slots:
    void onOrderCreateReceived(int status, const QString &message, const QString &orderId);
    void open();
    void onAddressUpdated(const QString &newAddress);

signals:
    void closeRequested();
    void addMenuRequested();
    void addressEditRequested();
    void orderSuccess();

private slots:
    void onCheckoutInfoReceived(int status, const QString &customerGrade,
                                int deliveryFee, int minOrderAmount,
                                const QString &pickupTime,
                                const QString &cardNumber,
                                const QString &accountNumber);
    // void onOrderCreateReceived(int status, const QString &message,
    //                            const QString &orderId);

    // ── 탭 ──
    void on_btnTabDelivery_clicked();
    void on_btnTabPickup_clicked();

    // ── 배달 버튼 ──
    void on_btnClose_clicked();
    void on_btnAddMenu_clicked();
    void on_btnAddressEdit_clicked();
    void on_btnPay_clicked();
    void on_btnRequestToggle_clicked();

    // ── 포장 버튼 ──
    void on_btnPickupAddMenu_clicked();
    void on_btnPickupRequestToggle_clicked();
    void on_btnPickupPay_clicked();
    void on_btnPickupCopyAddress_clicked();

    // ── 결제수단 토글 ──
    void on_btnPaymentExpand_clicked();
    void on_btnPickupPaymentExpand_clicked();

    // 라이더에게 요청사항 버튼
    void on_btnRiderRequest_clicked();

private:
    Ui::CartWidget *ui;
    NetworkManager *m_network;

    // ── 서버 데이터 ──
    QString m_customerGrade;
    int     m_deliveryFee    = 0;
    int     m_minOrderAmount = 0;

    // ── 포장/결제 서버 데이터 ──
    QString m_pickupTime = "";
    QString m_pickupStoreAddress = "";
    QString m_cardNumber;     // 결제수단: 카드번호
    QString m_accountNumber;  // 결제수단: 계좌번호
    QString m_selectedAddress;
    bool    m_paymentExpanded       = false;
    bool    m_pickupPaymentExpanded = false;

    // ── 모드 ──
    bool m_isPickupMode          = false;
    bool m_requestExpanded       = true;
    bool m_pickupRequestExpanded = true;
    bool m_serverDataLoaded      = false;

    // ── 배달 UI 업데이트 ──
    void updateAddress();
    void updateDeliverySection();
    void rebuildMenuList();
    void updatePriceSection();

    // ── 포장 UI 업데이트 ──
    void updatePickupInfo();
    void rebuildPickupMenuList();
    void updatePickupPriceSection();

    // ── 공통 하단 바 ──
    void updateBottomBar();

    // ── 메뉴 카드 (인덱스 기반, isPickup으로 콜백 구분) ──
    QWidget* makeMenuCard(int index, bool isPickup = false);

    // ── 결제수단 섹션 업데이트 ──
    void updatePaymentSection();

    // ── 계산 헬퍼 ──
    int  calcDeliveryFee() const;
    int  calcDiscount()    const;
    int  calcTotal()       const;
    int  calcPickupTotal() const;
    bool isMinOrderMet()   const;

    QString m_riderRequest = "문 앞에 놔주세요 (초인종 O)";

    int m_selectedDeliveryIndex = 0;
    int m_originalDeliveryFee = 0;
    bool eventFilter(QObject *obj, QEvent *event) override;

};
