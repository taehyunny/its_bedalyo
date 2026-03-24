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

    void open();

signals:
    void closeRequested();
    void addMenuRequested();
    void addressEditRequested();
    void orderSuccess();

private slots:
    void onCheckoutInfoReceived(int status, const QString &customerGrade,
                                int deliveryFee, int minOrderAmount);
    void onOrderCreateReceived(int status, const QString &message,
                               const QString &orderId);

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

private:
    Ui::CartWidget *ui;
    NetworkManager *m_network;

    // ── 서버 데이터 ──
    QString m_customerGrade;
    int     m_deliveryFee    = 0;
    int     m_minOrderAmount = 0;

    // ── 포장 서버 데이터 (추후 checkout DTO 확장 시 채워짐) ──
    QString m_pickupTime         = "15~25분";  // TODO: 서버에서 받아오기
    QString m_pickupStoreAddress = "";         // TODO: 서버에서 받아오기

    // ── 모드 ──
    bool m_isPickupMode          = false;
    bool m_requestExpanded       = true;
    bool m_pickupRequestExpanded = true;

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

    // ── 메뉴 카드 (배달/포장 공용, isPickup으로 람다 콜백 구분) ──
    QWidget* makeMenuCard(const CartItemQt &item, bool isPickup = false);

    // ── 계산 헬퍼 ──
    int  calcDeliveryFee() const;
    int  calcDiscount()    const;
    int  calcTotal()       const;
    int  calcPickupTotal() const;
    bool isMinOrderMet()   const;
};
