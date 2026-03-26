#include "cartwidget.h"
#include "ui_cartwidget.h"
#include "UserSession.h"
#include "storeutils.h"
#include <QDebug>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

// ============================================================
// 생성자
// ============================================================
CartWidget::CartWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 탭 ──
    connect(ui->btnTabDelivery, &QPushButton::clicked, this, &CartWidget::on_btnTabDelivery_clicked);
    connect(ui->btnTabPickup,   &QPushButton::clicked, this, &CartWidget::on_btnTabPickup_clicked);

    // ── 배달 버튼 ──
    connect(ui->btnClose,         &QPushButton::clicked, this, &CartWidget::on_btnClose_clicked);
    connect(ui->btnAddMenu,       &QPushButton::clicked, this, &CartWidget::on_btnAddMenu_clicked);
    connect(ui->btnAddressEdit,   &QPushButton::clicked, this, &CartWidget::on_btnAddressEdit_clicked);
    // connect(ui->btnPay,           &QPushButton::clicked, this, &CartWidget::on_btnPay_clicked);
    connect(ui->btnRequestToggle, &QPushButton::clicked, this, &CartWidget::on_btnRequestToggle_clicked);

    // ── 포장 버튼 ──
    connect(ui->btnPickupAddMenu,       &QPushButton::clicked, this, &CartWidget::on_btnPickupAddMenu_clicked);
    connect(ui->btnPickupRequestToggle, &QPushButton::clicked, this, &CartWidget::on_btnPickupRequestToggle_clicked);
    connect(ui->btnPickupPay,           &QPushButton::clicked, this, &CartWidget::on_btnPickupPay_clicked);
    connect(ui->btnPickupCopyAddress,   &QPushButton::clicked, this, &CartWidget::on_btnPickupCopyAddress_clicked);

    // ── 결제수단 토글 ──
    connect(ui->btnPaymentExpand,       &QPushButton::clicked, this, &CartWidget::on_btnPaymentExpand_clicked);
    connect(ui->btnPickupPaymentExpand, &QPushButton::clicked, this, &CartWidget::on_btnPickupPaymentExpand_clicked);

    // ── 네트워크 ──
    connect(m_network, &NetworkManager::onCheckoutInfoReceived,
            this, &CartWidget::onCheckoutInfoReceived);
    connect(m_network, &NetworkManager::onOrderCreateReceived,
            this, &CartWidget::onOrderCreateReceived);

    hide();
}

CartWidget::~CartWidget() { delete ui; }

// ============================================================
// 진입점
// ============================================================
void CartWidget::open()
{

    // 디버그로 확인
    qDebug() << "[CartWidget] open() storeName:" << CartSession::instance().storeName
             << "storeId:" << CartSession::instance().storeId;

    // 탭 상태 초기화 → 배달 탭 활성
    m_isPickupMode = false;
    ui->contentStack->setCurrentIndex(0);
    ui->btnTabDelivery->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid #111111;"
        "font-size:15px;font-weight:bold;color:#111111;padding:0 20px;}");
    ui->btnTabPickup->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid transparent;"
        "font-size:15px;color:#aaaaaa;padding:0 20px;}");

    // 주소
    updateAddress();

    // 가게명 (배달 + 포장 공용)
    QString storeName = CartSession::instance().storeName.isEmpty()
                        ? "-" : CartSession::instance().storeName;
    ui->lblStoreName->setText(storeName);
    ui->lblPickupStoreName->setText(storeName);

    // 배달 옵션 초기화
    QLayout *optLayout = ui->deliveryOptionsContainer->layout();
    if (optLayout) {
        QLayoutItem *child;
        while ((child = optLayout->takeAt(0)) != nullptr) {
            if (child->widget()) delete child->widget();
            delete child;
        }
    }
    ui->lblDeliveryLoading->show();

    // 서버 데이터 초기화
    m_customerGrade.clear();
    m_deliveryFee    = 0;
    m_minOrderAmount = 0;

    // 메뉴 / 가격 / 하단바 초기화
    rebuildMenuList();
    rebuildPickupMenuList();
    updatePriceSection();
    updatePickupPriceSection();
    updateBottomBar();

    // 요청사항 기본 펼침
    m_requestExpanded       = true;
    m_pickupRequestExpanded = true;
    ui->requestBody->show();
    ui->btnRequestToggle->setText("∧");
    ui->pickupRequestBody->show();
    ui->btnPickupRequestToggle->setText("∧");

    // 서버에 결제 정보 요청
    m_network->sendCheckoutInfo(UserSession::instance().userId,
                                CartSession::instance().storeId);
}

// ============================================================
// 탭 전환
// ============================================================
void CartWidget::on_btnTabDelivery_clicked()
{
    if (!m_isPickupMode) return;
    m_isPickupMode = false;
    ui->contentStack->setCurrentIndex(0);
    ui->btnTabDelivery->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid #111111;"
        "font-size:15px;font-weight:bold;color:#111111;padding:0 20px;}");
    ui->btnTabPickup->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid transparent;"
        "font-size:15px;color:#aaaaaa;padding:0 20px;}");
    // 배달 탭으로 올 때 포장쪽 변경사항 동기화
    rebuildMenuList();
    updatePriceSection();
    updateBottomBar();
}

void CartWidget::on_btnTabPickup_clicked()
{
    if (m_isPickupMode) return;
    m_isPickupMode = true;
    ui->contentStack->setCurrentIndex(1);
    ui->btnTabPickup->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid #111111;"
        "font-size:15px;font-weight:bold;color:#111111;padding:0 20px;}");
    ui->btnTabDelivery->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:2.5px solid transparent;"
        "font-size:15px;color:#aaaaaa;padding:0 20px;}");
    updatePickupInfo();
    // 포장 탭으로 올 때 배달쪽 변경사항 동기화
    rebuildPickupMenuList();
    updatePickupPriceSection();
    updateBottomBar();
}

// ============================================================
// 배달: 주소 라벨 업데이트
// ============================================================
void CartWidget::updateAddress()
{
    ui->lblAddress->setText(UserSession::instance().address.isEmpty()
                            ? "주소를 설정해주세요"
                            : UserSession::instance().address);
}

// ============================================================
// 배달: 배달 방법 동적 카드 생성 (서버 응답 후 호출)
// ============================================================
void CartWidget::updateDeliverySection()
{
    ui->lblDeliveryLoading->hide();

    QVBoxLayout *optLayout = qobject_cast<QVBoxLayout*>(
        ui->deliveryOptionsContainer->layout());
    if (!optLayout) return;

    bool isWow = (m_customerGrade == "와우");

    struct DeliveryOption {
        QString name;
        QString timeRange;
        int     fee;
        bool    isSelected;
        bool    hasSaveFee;
    };

    QList<DeliveryOption> options;
    if (isWow) {
        options.append({"와우 배달비 혜택", "31~46분", 0,            true,  false});
        options.append({"한집배달",         "29~39분", m_deliveryFee, false, false});
    } else {
        int saveFee = qMax(0, m_deliveryFee - 700);
        options.append({"한집배달",   "17~32분", m_deliveryFee, true,  false});
        options.append({"세이브배달", "22~37분", saveFee,       false, true});
    }

    for (const DeliveryOption &opt : options) {
        QWidget *card = new QWidget();
        card->setStyleSheet(opt.isSelected
            ? "QWidget{border:2px solid #1565c0;border-radius:8px;background:#f0f4ff;}"
            : "QWidget{border:1px solid #dddddd;border-radius:8px;background:#ffffff;}");

        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(12, 10, 12, 10);
        hl->setSpacing(10);

        QLabel *radio = new QLabel(opt.isSelected ? "●" : "○");
        radio->setStyleSheet(opt.isSelected
            ? "font-size:14px;color:#1565c0;background:transparent;"
            : "font-size:14px;color:#aaaaaa;background:transparent;");
        radio->setFixedWidth(16);

        QVBoxLayout *textVl = new QVBoxLayout();
        textVl->setSpacing(2);

        QLabel *nameLabel = new QLabel(opt.name);
        nameLabel->setStyleSheet(opt.isSelected
            ? "font-size:14px;font-weight:bold;color:#1565c0;background:transparent;"
            : "font-size:14px;color:#333333;background:transparent;");

        QLabel *timeLabel = new QLabel(opt.timeRange);
        timeLabel->setStyleSheet("font-size:12px;color:#888888;background:transparent;");

        textVl->addWidget(nameLabel);
        textVl->addWidget(timeLabel);

        // 요금 표시
        QWidget *feeWidget = new QWidget();
        feeWidget->setStyleSheet("background:transparent;");
        QVBoxLayout *feeVl = new QVBoxLayout(feeWidget);
        feeVl->setContentsMargins(0, 0, 0, 0);
        feeVl->setSpacing(0);
        feeVl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (opt.hasSaveFee && m_deliveryFee > 0) {
            QLabel *origFee = new QLabel(StoreUtils::formatWon(m_deliveryFee));
            origFee->setStyleSheet("font-size:12px;color:#aaaaaa;background:transparent;text-decoration:line-through;");
            origFee->setAlignment(Qt::AlignRight);
            QLabel *discFee = new QLabel(StoreUtils::formatWon(opt.fee));
            discFee->setStyleSheet("font-size:14px;color:#333333;background:transparent;");
            discFee->setAlignment(Qt::AlignRight);
            feeVl->addWidget(origFee);
            feeVl->addWidget(discFee);
        } else {
            QLabel *feeLabel = new QLabel(opt.fee == 0 ? "0원" : StoreUtils::formatWon(opt.fee));
            feeLabel->setStyleSheet(opt.fee == 0
                ? "font-size:14px;font-weight:bold;color:#1565c0;background:transparent;"
                : "font-size:14px;color:#333333;background:transparent;");
            feeLabel->setAlignment(Qt::AlignRight);
            feeVl->addWidget(feeLabel);
        }

        hl->addWidget(radio);
        hl->addLayout(textVl, 1);
        hl->addWidget(feeWidget);
        optLayout->addWidget(card);
    }

    if (isWow) {
        QLabel *hint = new QLabel("한집배달은 곧바로 고객님께 배달돼요");
        hint->setStyleSheet("font-size:12px;color:#888888;padding:2px 0;");
        optLayout->addWidget(hint);
    }
}

// ============================================================
// 포장: 매장 정보 업데이트 (탭 전환 시 / 서버 응답 시 호출)
// ============================================================
void CartWidget::updatePickupInfo()
{
    // CartSession에 저장된 정보를 멤버 변수에 업데이트
    m_pickupStoreAddress = CartSession::instance().storeAddress;
    m_pickupTime         = CartSession::instance().deliveryTimeRange;

    // UI 라벨에 적용
    ui->lblPickupTime->setText(m_pickupTime.isEmpty() ? "15~25분" : m_pickupTime);
    ui->lblPickupStoreAddress->setText(m_pickupStoreAddress.isEmpty()
                                           ? "주소 정보를 불러오는 중..."
                                           : m_pickupStoreAddress);
}

// ============================================================
// 메뉴 카드 (배달/포장 공용)
// isPickup=true 이면 pickupMenuListContainer에 rebuildPickupMenuList가 넣음
// ============================================================
QWidget* CartWidget::makeMenuCard(int index, bool isPickup)
{
    const CartItemQt &item = CartSession::instance().items[index];

    QWidget *card = new QWidget();
    card->setStyleSheet("QWidget{background:#ffffff;border-bottom:1px solid #f0f0f0;}");

    QHBoxLayout *hl = new QHBoxLayout(card);
    hl->setContentsMargins(16, 12, 16, 12);
    hl->setSpacing(12);

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setSpacing(2);

    // 메뉴명
    QLabel *nameLabel = new QLabel(item.menuName);
    nameLabel->setStyleSheet("font-size:14px;font-weight:bold;color:#111111;");
    vl->addWidget(nameLabel);

    // 옵션명 (있을 때만 표시)
    if (!item.optionName.isEmpty()) {
        QLabel *optLabel = new QLabel(item.optionName);
        optLabel->setStyleSheet("font-size:12px;color:#888888;");
        optLabel->setWordWrap(true);
        vl->addWidget(optLabel);
    }

    // 단가
    QLabel *priceLabel = new QLabel(StoreUtils::formatWon(item.unitPrice));
    priceLabel->setStyleSheet("font-size:13px;color:#555555;");
    vl->addWidget(priceLabel);

    hl->addLayout(vl, 1);

    QWidget *qtyWidget = new QWidget();
    qtyWidget->setStyleSheet("background:transparent;");
    QHBoxLayout *ql = new QHBoxLayout(qtyWidget);
    ql->setContentsMargins(0, 0, 0, 0);
    ql->setSpacing(0);

    QPushButton *btnMinus = new QPushButton(item.quantity == 1 ? "🗑" : "−");
    btnMinus->setFixedSize(32, 32);
    btnMinus->setStyleSheet(
        "QPushButton{background:#f0f0f0;border-radius:16px;border:none;font-size:14px;color:#333333;}"
        "QPushButton:hover{background:#e0e0e0;}");
    connect(btnMinus, &QPushButton::clicked, this,
            [this, index, qty = item.quantity, isPickup]() {
        CartSession::instance().updateQuantityByIndex(index, qty - 1);
        if (isPickup) { rebuildPickupMenuList(); updatePickupPriceSection(); }
        else          { rebuildMenuList();        updatePriceSection(); }
        updateBottomBar();
    });

    QLabel *lblQty = new QLabel(QString::number(item.quantity));
    lblQty->setFixedWidth(32);
    lblQty->setAlignment(Qt::AlignCenter);
    lblQty->setStyleSheet("font-size:14px;font-weight:bold;color:#111111;");

    QPushButton *btnPlus = new QPushButton("+");
    btnPlus->setFixedSize(32, 32);
    btnPlus->setStyleSheet(
        "QPushButton{background:#1565c0;border-radius:16px;border:none;font-size:18px;color:#ffffff;}"
        "QPushButton:hover{background:#1976d2;}");
    connect(btnPlus, &QPushButton::clicked, this,
            [this, index, qty = item.quantity, isPickup]() {
        CartSession::instance().updateQuantityByIndex(index, qty + 1);
        if (isPickup) { rebuildPickupMenuList(); updatePickupPriceSection(); }
        else          { rebuildMenuList();        updatePriceSection(); }
        updateBottomBar();
    });

    ql->addWidget(btnMinus);
    ql->addWidget(lblQty);
    ql->addWidget(btnPlus);
    hl->addWidget(qtyWidget);

    return card;
}

// ============================================================
// 배달: 메뉴 리스트 재빌드
// ============================================================
void CartWidget::rebuildMenuList()
{
    QVBoxLayout *menuLayout = qobject_cast<QVBoxLayout*>(ui->menuListContainer->layout());
    if (!menuLayout) return;

    QLayoutItem *child;
    while ((child = menuLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    for (int i = 0; i < CartSession::instance().items.size(); i++)
        menuLayout->addWidget(makeMenuCard(i, false));

    if (m_minOrderAmount > 0 && !isMinOrderMet()) {
        int diff = m_minOrderAmount - CartSession::instance().totalPrice();
        ui->lblMinOrder->setText(StoreUtils::formatWon(diff) + " 더 담으면 주문 가능");
        ui->lblMinOrder->show();
    } else {
        ui->lblMinOrder->hide();
    }
}

// ============================================================
// 포장: 메뉴 리스트 재빌드
// ============================================================
void CartWidget::rebuildPickupMenuList()
{
    QVBoxLayout *menuLayout = qobject_cast<QVBoxLayout*>(ui->pickupMenuListContainer->layout());
    if (!menuLayout) return;

    QLayoutItem *child;
    while ((child = menuLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    for (int i = 0; i < CartSession::instance().items.size(); i++)
        menuLayout->addWidget(makeMenuCard(i, true));

    if (m_minOrderAmount > 0 && !isMinOrderMet()) {
        int diff = m_minOrderAmount - CartSession::instance().totalPrice();
        ui->lblPickupMinOrder->setText(StoreUtils::formatWon(diff) + " 더 담으면 주문 가능");
        ui->lblPickupMinOrder->show();
    } else {
        ui->lblPickupMinOrder->hide();
    }
}

// ============================================================
// 배달: 결제금액 섹션 업데이트
// ============================================================
void CartWidget::updatePriceSection()
{
    bool isWow      = (m_customerGrade == "와우");
    int  orderPrice = CartSession::instance().totalPrice();
    int  discount   = calcDiscount();
    int  total      = calcTotal();

    if (isWow) {
        ui->lblDeliveryFeeTitle->setText("와우 전용 배달비");
        ui->lblDeliveryFeeTitle->setStyleSheet("font-size:14px;color:#1565c0;");
        ui->lblDeliveryFee->setText("0원");
        ui->lblDeliveryFee->setStyleSheet("font-size:14px;font-weight:bold;color:#1565c0;");
    } else {
        ui->lblDeliveryFeeTitle->setText("배달비");
        ui->lblDeliveryFeeTitle->setStyleSheet("font-size:14px;color:#555555;");
        ui->lblDeliveryFee->setText(StoreUtils::formatWon(m_deliveryFee));
        ui->lblDeliveryFee->setStyleSheet("font-size:14px;color:#111111;");
    }

    ui->lblOrderPrice->setText(StoreUtils::formatWon(orderPrice));
    ui->discountRow->setVisible(isWow);
    if (isWow && discount > 0)
        ui->lblDiscount->setText("- " + StoreUtils::formatWon(discount));

    ui->lblTotalPrice->setText(StoreUtils::formatWon(total));

    if (isWow && discount > 0) {
        ui->lblWowBadge->setText(
            QString("와우회원 배달비 0원 + %1 즉시할인 적용!")
                .arg(StoreUtils::formatWon(discount)));
        ui->lblWowBadge->show();
    } else {
        ui->lblWowBadge->hide();
    }
}

// ============================================================
// 포장: 결제금액 섹션 업데이트 (주문금액 = 총 결제금액, 배달비 없음)
// ============================================================
void CartWidget::updatePickupPriceSection()
{
    int total = calcPickupTotal();
    ui->lblPickupOrderPrice->setText(StoreUtils::formatWon(total));
    ui->lblPickupTotalPrice->setText(StoreUtils::formatWon(total));
}

// ============================================================
// 하단 바 업데이트 (배달/포장 모드 분기)
// ============================================================
void CartWidget::updateBottomBar()
{
    bool minMet = isMinOrderMet();

    if (m_isPickupMode) {
        // ── 포장 모드: 풀 너비 버튼 ──
        ui->deliveryBarContent->hide();
        ui->wowBottomBanner->hide();
        ui->btnPickupPay->show();

        ui->btnPickupPay->setEnabled(minMet && !CartSession::instance().isEmpty());

        if (!minMet && m_minOrderAmount > 0) {
            ui->btnPickupPay->setText(
                StoreUtils::formatWon(m_minOrderAmount) + " 이상 주문 가능");
        } else {
            ui->btnPickupPay->setText(
                QString("포장주문 %1 결제하기")
                    .arg(StoreUtils::formatWon(calcPickupTotal())));
        }

    } else {
        // ── 배달 모드 ──
        ui->btnPickupPay->hide();
        ui->deliveryBarContent->show();

        bool isWow    = (m_customerGrade == "와우");
        int  total    = calcTotal();
        int  discount = calcDiscount();
        int  original = CartSession::instance().totalPrice() + m_deliveryFee;

        // WOW 배너
        if (isWow && discount > 0) {
            ui->wowBottomBanner->show();
            ui->lblWowBottomText->setText(
                QString("WOW! 와우회원 배달비 0원 + %1 즉시할인 적용 ✨")
                    .arg(StoreUtils::formatWon(discount)));
        } else {
            ui->wowBottomBanner->hide();
        }

        ui->btnPay->setEnabled(minMet && !CartSession::instance().isEmpty());

        if (!minMet && m_minOrderAmount > 0) {
            ui->btnPay->setText(StoreUtils::formatWon(m_minOrderAmount) + " 이상 주문 가능");
            ui->lblPayFinal->setText("−");
            ui->lblPayOriginal->hide();
        } else {
            ui->btnPay->setText("배달주문 결제하기");
            ui->lblPayFinal->setText(StoreUtils::formatWon(total));

            if (isWow && m_deliveryFee > 0 && discount > 0) {
                ui->lblPayOriginal->setText(StoreUtils::formatWon(original));
                ui->lblPayOriginal->show();
            } else {
                ui->lblPayOriginal->hide();
            }
        }
    }
}

// ============================================================
// 서버 응답: 결제 정보 수신
// ============================================================
// 파라미터를 4개로 줄이고, storeAddress/pickupTime은 CartSession에서 꺼냄
void CartWidget::onCheckoutInfoReceived(int status, const QString &customerGrade,
                                        int deliveryFee, int minOrderAmount,
                                        const QString &pickupTime,
                                        const QString &cardNumber,
                                        const QString &accountNumber)
{
    if (status != 200) {
        qWarning() << "[CartWidget] 결제 정보 수신 실패 status:" << status;
        return;
    }

    m_customerGrade  = customerGrade;
    m_deliveryFee    = deliveryFee;
    m_minOrderAmount = minOrderAmount;

    // 포장 시간 / 결제수단: 서버에서 직접 받음
    if (!pickupTime.isEmpty())   m_pickupTime    = pickupTime;
    if (!cardNumber.isEmpty())   m_cardNumber    = cardNumber;
    if (!accountNumber.isEmpty()) m_accountNumber = accountNumber;

    // storeAddress는 CartSession에서 (RES_STORE_DETAIL 때 저장됨)
    m_pickupStoreAddress = CartSession::instance().storeAddress;

    qDebug() << "[CartWidget] grade:" << customerGrade
             << "fee:" << deliveryFee
             << "pickupTime:" << m_pickupTime;

    updateDeliverySection();
    updatePriceSection();
    updatePickupInfo();
    updatePickupPriceSection();
    updatePaymentSection();
    updateBottomBar();
    rebuildMenuList();
    rebuildPickupMenuList();
}

// ============================================================
// 서버 응답: 주문 생성 완료
// ============================================================
void CartWidget::onOrderCreateReceived(int status, const QString &message,
                                       const QString &orderId)
{
    Q_UNUSED(orderId)
    if (status == 200) {
        CartSession::instance().clear();
        QString msg = m_isPickupMode ? "포장 주문이 완료되었습니다!" : "주문이 완료되었습니다!";
        QMessageBox::information(this, "주문 완료", msg);
        emit orderSuccess();
    } else {
        QMessageBox::warning(this, "주문 실패", message);
    }
}

// ============================================================
// 계산 헬퍼
// ============================================================
int  CartWidget::calcDeliveryFee() const { return (m_customerGrade == "와우") ? 0 : m_deliveryFee; }
int  CartWidget::calcDiscount()    const { return (m_customerGrade == "와우") ? m_deliveryFee : 0; }
int  CartWidget::calcTotal()       const { return CartSession::instance().totalPrice() + calcDeliveryFee(); }
int  CartWidget::calcPickupTotal() const { return CartSession::instance().totalPrice(); } // 배달비 없음
bool CartWidget::isMinOrderMet()   const {
    if (m_minOrderAmount <= 0) return true;
    return CartSession::instance().totalPrice() >= m_minOrderAmount;
}

// ============================================================
// 배달 버튼 슬롯
// ============================================================
void CartWidget::on_btnClose_clicked()       { emit closeRequested(); }
void CartWidget::on_btnAddMenu_clicked()     { emit addMenuRequested(); }
void CartWidget::on_btnAddressEdit_clicked() { emit addressEditRequested(); }

void CartWidget::on_btnRequestToggle_clicked()
{
    m_requestExpanded = !m_requestExpanded;
    ui->requestBody->setVisible(m_requestExpanded);
    ui->btnRequestToggle->setText(m_requestExpanded ? "∧" : "∨");
}

void CartWidget::on_btnPay_clicked()
{
    static int callCount = 0;
    qDebug() << "[CartWidget] 결제 버튼 클릭됨! 횟수:" << ++callCount;

    if (!isMinOrderMet() || CartSession::instance().isEmpty()) return;

    OrderCreateReqDTO dto;
    dto.userId          = UserSession::instance().userId.toStdString();
    dto.storeId         = CartSession::instance().storeId;
    dto.totalPrice      = calcTotal();
    dto.deliveryAddress = UserSession::instance().address.toStdString();
    dto.couponId        = -1;

    for (const CartItemQt &item : CartSession::instance().items) {
        OrderItemDTO orderItem;
        orderItem.menuId          = item.menuId;
        orderItem.quantity        = item.quantity;
        orderItem.unitPrice       = item.unitPrice;
        orderItem.menuName        = item.menuName.toStdString();
        nlohmann::json optArr = nlohmann::json::array();
        for (int id : item.optionIds) optArr.push_back(id);
        orderItem.selectedOptions = optArr;
        dto.items.push_back(orderItem);
    }
    dto.storeRequest = ui->pickupStoreRequestEdit->toPlainText().toStdString();

    qDebug() << "====== [배달주문 결제하기] ======";
    qDebug() << "userId         :" << QString::fromStdString(dto.userId);
    qDebug() << "storeId        :" << dto.storeId;
    qDebug() << "totalPrice     :" << dto.totalPrice;
    qDebug() << "deliveryAddress:" << QString::fromStdString(dto.deliveryAddress);
    qDebug() << "couponId       :" << dto.couponId;
    qDebug() << "── 메뉴 목록 ──";
    for (const auto &item : dto.items) {
        qDebug() << "  menuId:" << item.menuId
                 << "qty:"      << item.quantity
                 << "price:"    << item.unitPrice;
    }
    qDebug() << "=================================";

    m_network->sendOrderCreate(dto);
}

// ============================================================
// 결제수단 섹션 업데이트
// ============================================================
void CartWidget::updatePaymentSection()
{
    QString cardText    = m_cardNumber.isEmpty()    ? "카드 정보 없음" : m_cardNumber;
    QString accountText = m_accountNumber.isEmpty() ? "계좌 정보 없음" : m_accountNumber;
    ui->lblCardNumber->setText("💳 " + cardText);
    ui->lblAccountNumber->setText("🏦 " + accountText);
    // 포장 탭에도 동일하게
    ui->lblPickupCardNumber->setText("💳 " + cardText);
    ui->lblPickupAccountNumber->setText("🏦 " + accountText);
}

void CartWidget::on_btnPaymentExpand_clicked()
{
    m_paymentExpanded = !m_paymentExpanded;
    ui->paymentBody->setVisible(m_paymentExpanded);
    ui->btnPaymentExpand->setText(m_paymentExpanded ? "∧" : "∨");
}

void CartWidget::on_btnPickupPaymentExpand_clicked()
{
    m_pickupPaymentExpanded = !m_pickupPaymentExpanded;
    ui->pickupPaymentBody->setVisible(m_pickupPaymentExpanded);
    ui->btnPickupPaymentExpand->setText(m_pickupPaymentExpanded ? "∧" : "∨");
}

// ============================================================
// 포장 버튼 슬롯
// ============================================================
void CartWidget::on_btnPickupAddMenu_clicked()     { emit addMenuRequested(); }

void CartWidget::on_btnPickupRequestToggle_clicked()
{
    m_pickupRequestExpanded = !m_pickupRequestExpanded;
    ui->pickupRequestBody->setVisible(m_pickupRequestExpanded);
    ui->btnPickupRequestToggle->setText(m_pickupRequestExpanded ? "∧" : "∨");
}

void CartWidget::on_btnPickupCopyAddress_clicked()
{
    // TODO: 기능 구현 예정
    QApplication::clipboard()->setText(m_pickupStoreAddress);
    qDebug() << "[CartWidget] 주소 복사됨:" << m_pickupStoreAddress;
}

void CartWidget::on_btnPickupPay_clicked()
{
    if (!isMinOrderMet() || CartSession::instance().isEmpty()) return;

    OrderCreateReqDTO dto;
    dto.userId          = UserSession::instance().userId.toStdString();
    dto.storeId         = CartSession::instance().storeId;
    dto.totalPrice      = calcPickupTotal();
    dto.deliveryAddress = "";   // 포장은 배달 주소 없음
    dto.couponId        = -1;

    for (const CartItemQt &item : CartSession::instance().items) {
        OrderItemDTO orderItem;
        orderItem.menuId          = item.menuId;
        orderItem.quantity        = item.quantity;
        orderItem.unitPrice       = item.unitPrice;
        orderItem.selectedOptions = nlohmann::json::array();
        dto.items.push_back(orderItem);
    }
    m_network->sendOrderCreate(dto);
}
