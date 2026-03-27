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

    // 2. 가게 이름 & 주문번호/일시 정보
    QLabel* lblStore = new QLabel(QString::fromStdString(data.storeName));
    lblStore->setStyleSheet("font-size: 20px; font-weight: bold; color: #111111;");
    lblStore->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblStore);

    // 주문번호(orderId)와 일시(createdAt)를 정확히 매칭
    QLabel* lblOrderInfo = new QLabel(QString("주문번호: %1\n주문일시: %2")
                                      .arg(QString::fromStdString(data.orderId))   // ORD-... 형태
                                      .arg(QString::fromStdString(data.createdAt))); // 날짜/시간 형태
    lblOrderInfo->setStyleSheet("font-size: 13px; color: #777777;");
    lblOrderInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblOrderInfo);
    mainLayout->addSpacing(10);

    // 3. 메뉴 리스트 출력 부분 수정
    for (const auto& item : data.items) {
        QHBoxLayout* itemRow = new QHBoxLayout();
        
        // [수정] 서버 DTO 구조(OrderDTO.h)에 맞춰 menuName을 가져옵니다.
        QString menuName = "알 수 없는 메뉴";
        
        // OrderItemDTO에는 menuName 멤버가 직접 존재합니다.
        if (!item.menuName.empty()) {
            menuName = QString::fromStdString(item.menuName);
        } 
        // 만약 menuName이 비어있다면 selectedOptions 내부를 마지막으로 확인
        else if (item.selectedOptions.contains("menuName") && item.selectedOptions["menuName"].is_string()) {
            menuName = QString::fromStdString(item.selectedOptions["menuName"].get<std::string>());
        }

        QLabel* nameLabel = new QLabel(menuName);
        nameLabel->setStyleSheet("font-size: 15px;");
        nameLabel->setWordWrap(true);

        // 가격 계산 (단가 * 수량)
        QLabel* priceLabel = new QLabel(QLocale(QLocale::Korean).toString(item.unitPrice * item.quantity) + "원");
        priceLabel->setStyleSheet("font-size: 15px;");
        priceLabel->setAlignment(Qt::AlignRight);

        itemRow->addWidget(nameLabel);
        itemRow->addWidget(priceLabel);
        mainLayout->addLayout(itemRow);

        // 메뉴 밑에 "└ 옵션들" 표시
        if (!item.options.empty()) {
            QString optText = " └ ";
            for (const auto& opt : item.options) {
                optText += QString::fromStdString(opt) + ", ";
            }
            optText.chop(2); // 마지막 쉼표 제거
            QLabel* lblOpt = new QLabel(optText);
            lblOpt->setStyleSheet("font-size: 13px; color: #888888; margin-left: 10px;");
            mainLayout->addWidget(lblOpt);
        }
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

    mainLayout->addWidget(createLine());
    addSummaryRow("주문금액", QLocale(QLocale::Korean).toString(data.totalMenuPrice) + "원");
    addSummaryRow("배달비", QLocale(QLocale::Korean).toString(data.deliveryFee) + "원");

    // 할인 금액이 0보다 클 때만 영수증에 표시
    if (data.wowDiscount > 0) {
        addSummaryRow("와우회원 할인", "-" + QLocale(QLocale::Korean).toString(data.wowDiscount) + "원");
    }
    if (data.couponDiscount > 0) {
        addSummaryRow("쿠폰 할인", "-" + QLocale(QLocale::Korean).toString(data.couponDiscount) + "원");
    }

    mainLayout->addWidget(createLine());

    // 5. 총 결제 금액
    addSummaryRow(QString::fromStdString(data.paymentMethod), QLocale(QLocale::Korean).toString(data.totalPrice) + "원");
    mainLayout->addSpacing(10);
    addSummaryRow("총 결제금액", QLocale(QLocale::Korean).toString(data.totalPrice) + "원", true); // 볼드 강조
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