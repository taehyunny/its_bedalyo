#include "cartwidget.h"
#include "ui_cartwidget.h"   // uic가 cartwidget.ui로부터 자동 생성
#include "UserSession.h"
#include "storeutils.h"
#include <QDebug>
#include <QMessageBox>

// ============================================================
// 생성자
// ============================================================
CartWidget::CartWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 버튼 시그널 연결 ──
    connect(ui->btnClose,         &QPushButton::clicked, this, &CartWidget::on_btnClose_clicked);
    connect(ui->btnAddMenu,       &QPushButton::clicked, this, &CartWidget::on_btnAddMenu_clicked);
    connect(ui->btnAddressEdit,   &QPushButton::clicked, this, &CartWidget::on_btnAddressEdit_clicked);
    connect(ui->btnPay,           &QPushButton::clicked, this, &CartWidget::on_btnPay_clicked);
    connect(ui->btnRequestToggle, &QPushButton::clicked, this, &CartWidget::on_btnRequestToggle_clicked);

    // ── 네트워크 시그널 연결 ──
    // [오류 수정] NetworkManager.h에서 중복 시그널 제거 → 오버로딩 없어서 static_cast 불필요
    connect(m_network, &NetworkManager::onCheckoutInfoReceived,
            this, &CartWidget::onCheckoutInfoReceived);
    connect(m_network, &NetworkManager::onOrderCreateReceived,
            this, &CartWidget::onOrderCreateReceived);
}

CartWidget::~CartWidget()
{
    delete ui;
}

// ============================================================
// 진입점: MainWindow에서 장바구니 열 때 호출
// ============================================================
void CartWidget::open()
{
    // ── 주소 업데이트 ──
    updateAddress();

    // ── 가게명 업데이트 ──
    ui->lblStoreName->setText(CartSession::instance().storeName.isEmpty()
                              ? "-" : CartSession::instance().storeName);

    // ── 배달 섹션 초기화 (이전 옵션 카드 제거 + 로딩 표시) ──
    QLayout *optLayout = ui->deliveryOptionsContainer->layout();
    if (optLayout) {
        QLayoutItem *child;
        while ((child = optLayout->takeAt(0)) != nullptr) {
            if (child->widget()) delete child->widget();
            delete child;
        }
    }
    ui->lblDeliveryLoading->show();

    // ── 서버 데이터 초기화 ──
    m_customerGrade.clear();
    m_deliveryFee    = 0;
    m_minOrderAmount = 0;

    // ── 메뉴 / 가격 / 하단바 초기화 ──
    rebuildMenuList();
    updatePriceSection();
    updateBottomBar();

    // ── 요청사항 기본 펼침 ──
    m_requestExpanded = true;
    ui->requestBody->show();
    ui->btnRequestToggle->setText("∧");

    // ── 서버에 결제 정보 요청 ──
    m_network->sendCheckoutInfo(UserSession::instance().userId,
                                CartSession::instance().storeId);
}

// ============================================================
// 주소 라벨 업데이트
// ============================================================
void CartWidget::updateAddress()
{
    ui->lblAddress->setText(UserSession::instance().address.isEmpty()
                            ? "주소를 설정해주세요"
                            : UserSession::instance().address);
}

// ============================================================
// 배달 방법 섹션 - 동적 옵션 카드 생성 (서버 응답 후 호출)
// ============================================================
void CartWidget::updateDeliverySection()
{
    // 로딩 라벨 숨김
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
    };

    QList<DeliveryOption> options;
    if (isWow) {
        options.append({"와우 배달비 혜택", "31~46분", 0,            true});
        options.append({"한집배달",         "29~39분", m_deliveryFee, false});
    } else {
        options.append({"한집배달",         "21~31분", m_deliveryFee, true});
    }

    for (const DeliveryOption &opt : options) {
        QWidget *card = new QWidget();
        card->setStyleSheet(opt.isSelected
            ? "QWidget { border: 2px solid #1565c0; border-radius: 8px; background: #f0f4ff; }"
            : "QWidget { border: 1px solid #dddddd; border-radius: 8px; background: #ffffff; }"
        );

        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(12, 10, 12, 10);
        hl->setSpacing(10);

        QLabel *radio = new QLabel(opt.isSelected ? "●" : "○");
        radio->setStyleSheet(opt.isSelected
            ? "font-size: 14px; color: #1565c0; background: transparent;"
            : "font-size: 14px; color: #aaaaaa; background: transparent;");
        radio->setFixedWidth(16);

        QVBoxLayout *textVl = new QVBoxLayout();
        textVl->setSpacing(2);

        QLabel *nameLabel = new QLabel(opt.name);
        nameLabel->setStyleSheet(opt.isSelected
            ? "font-size: 14px; font-weight: bold; color: #1565c0; background: transparent;"
            : "font-size: 14px; color: #333333; background: transparent;");

        QLabel *timeLabel = new QLabel(opt.timeRange);
        timeLabel->setStyleSheet("font-size: 12px; color: #888888; background: transparent;");

        textVl->addWidget(nameLabel);
        textVl->addWidget(timeLabel);

        QLabel *feeLabel = new QLabel(opt.fee == 0 ? "0원" : StoreUtils::formatWon(opt.fee));
        feeLabel->setStyleSheet(opt.fee == 0
            ? "font-size: 14px; font-weight: bold; color: #1565c0; background: transparent;"
            : "font-size: 14px; color: #333333; background: transparent;");

        hl->addWidget(radio);
        hl->addLayout(textVl, 1);
        hl->addWidget(feeLabel);

        optLayout->addWidget(card);
    }

    if (isWow) {
        QLabel *hint = new QLabel("한집배달은 곧바로 고객님께 배달돼요");
        hint->setStyleSheet("font-size: 12px; color: #888888; padding: 2px 0;");
        optLayout->addWidget(hint);
    }
}

// ============================================================
// 메뉴 카드 (수량 조절 +/- 포함)
// ============================================================
QWidget* CartWidget::makeMenuCard(const CartItemQt &item)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "QWidget { background: #ffffff; border-bottom: 1px solid #f0f0f0; }");

    QHBoxLayout *hl = new QHBoxLayout(card);
    hl->setContentsMargins(16, 12, 16, 12);
    hl->setSpacing(12);

    // 메뉴명 + 가격
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setSpacing(3);

    QLabel *nameLabel = new QLabel(item.menuName);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #111111;");

    QLabel *priceLabel = new QLabel(StoreUtils::formatWon(item.unitPrice));
    priceLabel->setStyleSheet("font-size: 13px; color: #555555;");

    vl->addWidget(nameLabel);
    vl->addWidget(priceLabel);
    hl->addLayout(vl, 1);

    // 수량 조절 버튼 (− / 숫자 / +)
    QWidget *qtyWidget = new QWidget();
    qtyWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *ql = new QHBoxLayout(qtyWidget);
    ql->setContentsMargins(0, 0, 0, 0);
    ql->setSpacing(0);

    QPushButton *btnMinus = new QPushButton(item.quantity == 1 ? "🗑" : "−");
    btnMinus->setFixedSize(32, 32);
    btnMinus->setStyleSheet(
        "QPushButton { background: #f0f0f0; border-radius: 16px; border: none;"
        "  font-size: 14px; color: #333333; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(btnMinus, &QPushButton::clicked, this,
            [this, menuId = item.menuId, qty = item.quantity]() {
        CartSession::instance().updateQuantity(menuId, qty - 1);
        rebuildMenuList();
        updatePriceSection();
        updateBottomBar();
    });

    QLabel *lblQty = new QLabel(QString::number(item.quantity));
    lblQty->setFixedWidth(32);
    lblQty->setAlignment(Qt::AlignCenter);
    lblQty->setStyleSheet("font-size: 14px; font-weight: bold; color: #111111;");

    QPushButton *btnPlus = new QPushButton("+");
    btnPlus->setFixedSize(32, 32);
    btnPlus->setStyleSheet(
        "QPushButton { background: #1565c0; border-radius: 16px; border: none;"
        "  font-size: 18px; color: #ffffff; }"
        "QPushButton:hover { background: #1976d2; }"
    );
    connect(btnPlus, &QPushButton::clicked, this,
            [this, menuId = item.menuId, qty = item.quantity]() {
        CartSession::instance().updateQuantity(menuId, qty + 1);
        rebuildMenuList();
        updatePriceSection();
        updateBottomBar();
    });

    ql->addWidget(btnMinus);
    ql->addWidget(lblQty);
    ql->addWidget(btnPlus);
    hl->addWidget(qtyWidget);

    return card;
}

// ============================================================
// 메뉴 리스트 재빌드 (장바구니 변경 시 호출)
// ============================================================
void CartWidget::rebuildMenuList()
{
    QVBoxLayout *menuLayout = qobject_cast<QVBoxLayout*>(
        ui->menuListContainer->layout());
    if (!menuLayout) return;

    // 기존 카드 제거
    QLayoutItem *child;
    while ((child = menuLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    // 카드 재생성
    for (const CartItemQt &item : CartSession::instance().items)
        menuLayout->addWidget(makeMenuCard(item));

    // 최소주문금액 안내
    if (m_minOrderAmount > 0 && !isMinOrderMet()) {
        int diff = m_minOrderAmount - CartSession::instance().totalPrice();
        ui->lblMinOrder->setText(StoreUtils::formatWon(diff) + " 더 담으면 주문 가능");
        ui->lblMinOrder->show();
    } else {
        ui->lblMinOrder->hide();
    }
}

// ============================================================
// 결제금액 섹션 업데이트
// ============================================================
void CartWidget::updatePriceSection()
{
    int orderPrice  = CartSession::instance().totalPrice();
    int deliveryFee = calcDeliveryFee();
    int discount    = calcDiscount();
    int total       = calcTotal();

    ui->lblOrderPrice->setText(StoreUtils::formatWon(orderPrice));
    ui->lblDeliveryFee->setText(StoreUtils::formatWon(deliveryFee));

    if (discount > 0) {
        ui->lblDiscount->setText("- " + StoreUtils::formatWon(discount));
        ui->lblDiscount->setStyleSheet("font-size: 14px; color: #e53935;");
    } else {
        ui->lblDiscount->setText("0원");
        ui->lblDiscount->setStyleSheet("font-size: 14px; color: #111111;");
    }

    ui->lblTotalPrice->setText(StoreUtils::formatWon(total));

    // 와우 할인 배지
    if (m_customerGrade == "와우" && discount > 0) {
        ui->lblWowBadge->setText(
            QString("와우회원 배달비 0원 + %1 즉시할인 적용!")
                .arg(StoreUtils::formatWon(discount)));
        ui->lblWowBadge->show();
    } else {
        ui->lblWowBadge->hide();
    }
}

// ============================================================
// 하단 바 업데이트
// ============================================================
void CartWidget::updateBottomBar()
{
    int  total    = calcTotal();
    int  original = CartSession::instance().totalPrice() + m_deliveryFee;
    bool minMet   = isMinOrderMet();

    ui->btnPay->setEnabled(minMet && !CartSession::instance().isEmpty());

    if (!minMet && m_minOrderAmount > 0) {
        ui->btnPay->setText(StoreUtils::formatWon(m_minOrderAmount) + " 이상 주문 가능");
        ui->lblPayFinal->setText("−");
        ui->lblPayOriginal->hide();
    } else {
        ui->btnPay->setText("배달주문 결제하기");
        ui->lblPayFinal->setText(StoreUtils::formatWon(total));

        if (m_customerGrade == "와우" && m_deliveryFee > 0) {
            ui->lblPayOriginal->setText(StoreUtils::formatWon(original));
            ui->lblPayOriginal->show();
        } else {
            ui->lblPayOriginal->hide();
        }
    }
}

// ============================================================
// 서버 응답: 결제 정보 수신
// ============================================================
void CartWidget::onCheckoutInfoReceived(int status, const QString &customerGrade,
                                        int deliveryFee, int minOrderAmount)
{
    if (status != 200) {
        qWarning() << "[CartWidget] 결제 정보 수신 실패 status:" << status;
        return;
    }

    m_customerGrade  = customerGrade;
    m_deliveryFee    = deliveryFee;
    m_minOrderAmount = minOrderAmount;

    qDebug() << "[CartWidget] grade:" << customerGrade
             << "fee:" << deliveryFee
             << "min:" << minOrderAmount;

    updateDeliverySection();
    updatePriceSection();
    updateBottomBar();
    rebuildMenuList();
}

// ============================================================
// 서버 응답: 주문 생성 완료
// [오류 수정] const QString& 로 통일
// ============================================================
void CartWidget::onOrderCreateReceived(int status, const QString &message,
                                       const QString &orderId)
{
    Q_UNUSED(orderId)
    if (status == 200) {
        CartSession::instance().clear();
        QMessageBox::information(this, "주문 완료", "주문이 완료되었습니다!");
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
bool CartWidget::isMinOrderMet()   const
{
    if (m_minOrderAmount <= 0) return true;
    return CartSession::instance().totalPrice() >= m_minOrderAmount;
}

// ============================================================
// 버튼 슬롯
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
    if (!isMinOrderMet()) return;
    if (CartSession::instance().isEmpty()) return;

    OrderCreateReqDTO dto;
    dto.userId          = UserSession::instance().userId.toStdString();
    dto.storeId         = CartSession::instance().storeId;
    dto.totalPrice      = calcTotal();
    dto.deliveryAddress = UserSession::instance().address.toStdString();
    dto.couponId        = -1;

    for (const CartItemQt &item : CartSession::instance().items) {
        OrderItemDTO orderItem;
        orderItem.menuId           = item.menuId;
        orderItem.quantity         = item.quantity;
        orderItem.unitPrice        = item.unitPrice;
        orderItem.selectedOptions  = nlohmann::json::array();
        dto.items.push_back(orderItem);
    }

    m_network->sendOrderCreate(dto);
}
