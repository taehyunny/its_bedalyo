#include "ordercompletewidget.h"
#include "ui_ordercompletewidget.h"
#include <QLabel>

OrderCompleteWidget::OrderCompleteWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderCompleteWidget)
    , m_network(network)
{
    ui->setupUi(this);
}

OrderCompleteWidget::~OrderCompleteWidget() {
    delete ui;
}

// 🚀 동적 데이터 세팅 (주문번호, 가게명, 주소)
void OrderCompleteWidget::setOrderData(const QString& orderId, const QString& storeName, const QString& address) {
    ui->lblOrderId->setText(orderId + " 주문");
    ui->lblStoreName->setText(storeName);
    ui->lblAddressValue->setText(address);
}

// 🚀 메뉴 내역을 동적으로 하나씩 추가하는 함수 (장바구니 데이터를 받아서 그림)
void OrderCompleteWidget::addMenuItem(int quantity, const QString& menuName) {
    // 수량 라벨
    QLabel* lblQty = new QLabel(QString::number(quantity));
    lblQty->setFixedSize(22, 22);
    lblQty->setStyleSheet("background-color: #f5f5f5; color: #333; font-weight: bold; font-size: 13px;");
    lblQty->setAlignment(Qt::AlignCenter);

    // 메뉴 이름 라벨
    QLabel* lblName = new QLabel(menuName);
    lblName->setStyleSheet("font-size: 15px; color: #111111;");

    // 가로 레이아웃에 묶기
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(lblQty);
    hLayout->addWidget(lblName);
    hLayout->addStretch(); // 오른쪽 여백

    // UI의 세로 목록 레이아웃(menuListLayout)에 방금 만든 가로 한 줄을 추가!
    ui->menuListLayout->addLayout(hLayout);
}

// 🚀 이전에 그려진 메뉴가 있다면 싹 지우고 초기화
void OrderCompleteWidget::clearMenuItems() {
    QLayoutItem *child;
    while ((child = ui->menuListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}