#include "orderreceiptdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QLocale>

// 가로선 그어주는 헬퍼 함수
QFrame* createLine() {
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #E0E0E0;");
    return line;
}

OrderReceiptDialog::OrderReceiptDialog(const ResOrderDetailDTO& data, QWidget* parent)
    : QDialog(parent)
{
    // 창 설정 (배경 흐리게, 흰색 바탕)
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setFixedSize(380, 650);
    setStyleSheet("QDialog { background-color: white; border-radius: 12px; } QLabel { color: #333333; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 1. 상단 타이틀 바 (X 버튼 + 영수증)
    QHBoxLayout* topLayout = new QHBoxLayout();
    QPushButton* btnClose = new QPushButton("✕");
    btnClose->setStyleSheet("font-size: 20px; font-weight: bold; border: none; color: #555555;");
    btnClose->setFixedSize(30, 30);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept); // X 누르면 닫힘

    QLabel* lblTitle = new QLabel("영수증");
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    lblTitle->setAlignment(Qt::AlignCenter);

    topLayout->addWidget(btnClose);
    topLayout->addWidget(lblTitle, 1);
    topLayout->addSpacing(30); // 중앙 정렬을 위한 꼼수
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(createLine());

    // 2. 가게 이름 & 날짜
    QLabel* lblStore = new QLabel(QString::fromStdString(data.storeName));
    lblStore->setStyleSheet("font-size: 20px; font-weight: bold; color: #111111;");
    lblStore->setAlignment(Qt::AlignCenter);

    QLabel* lblDate = new QLabel("주문번호:\n" + QString::fromStdString(data.createdAt));
    lblDate->setStyleSheet("font-size: 14px; color: #555555;");
    lblDate->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(lblStore);
    mainLayout->addWidget(lblDate);
    mainLayout->addSpacing(10);

    // 3. 메뉴 리스트 출력
    for (const auto& item : data.items) {
        QHBoxLayout* itemRow = new QHBoxLayout();
        QString menuName = QString::fromStdString(item.selectedOptions.value("menuName", "알 수 없는 메뉴"));

        QLabel* nameLabel = new QLabel(menuName);
        nameLabel->setStyleSheet("font-size: 15px;");

        // 가격 계산 (단가 * 수량)
        QLabel* priceLabel = new QLabel(QLocale(QLocale::Korean).toString(item.unitPrice * item.quantity) + "원");
        priceLabel->setStyleSheet("font-size: 15px;");
        priceLabel->setAlignment(Qt::AlignRight);

        itemRow->addWidget(nameLabel);
        itemRow->addWidget(priceLabel);
        mainLayout->addLayout(itemRow);
    }
    mainLayout->addWidget(createLine());

    // 4. 결제 요약
    auto addSummaryRow = [&](const QString& leftText, const QString& rightText, bool isBold = false) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* left = new QLabel(leftText);
        QLabel* right = new QLabel(rightText);
        QString style = isBold ? "font-size: 15px; font-weight: bold;" : "font-size: 14px; color: #555555;";
        left->setStyleSheet(style);
        right->setStyleSheet(style);
        right->setAlignment(Qt::AlignRight);
        row->addWidget(left);
        row->addWidget(right);
        mainLayout->addLayout(row);
    };

    addSummaryRow("주문금액", QLocale(QLocale::Korean).toString(data.totalMenuPrice) + "원");
    addSummaryRow("배달비", QLocale(QLocale::Korean).toString(data.deliveryFee) + "원");
    mainLayout->addWidget(createLine());

    // 5. 총 결제 금액
    addSummaryRow(QString::fromStdString(data.paymentMethod) + " 결제", QLocale(QLocale::Korean).toString(data.totalPrice) + "원");
    mainLayout->addSpacing(10);
    addSummaryRow("총 결제금액", QLocale(QLocale::Korean).toString(data.totalPrice) + "원", true); // 볼드 처리
    mainLayout->addSpacing(20);

    // 6. 배달 주소
    QLabel* lblAddressTitle = new QLabel("(배달주소)");
    lblAddressTitle->setStyleSheet("font-size: 14px; color: #555555;");
    QLabel* lblAddress = new QLabel(QString::fromStdString(data.deliveryAddress));
    lblAddress->setStyleSheet("font-size: 15px;");
    lblAddress->setWordWrap(true);

    mainLayout->addWidget(lblAddressTitle);
    mainLayout->addWidget(lblAddress);

    mainLayout->addStretch();
}