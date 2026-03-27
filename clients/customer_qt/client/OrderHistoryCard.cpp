#include "OrderHistoryCard.h"
#include "StoreUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLocale>
#include <QMouseEvent>
#include <QFrame>

OrderHistoryCard::OrderHistoryCard(const OrderHistoryItemDTO& info, QWidget* parent)
    : QFrame(parent)
    , m_orderId(QString::fromStdString(info.orderId))
    , m_status(info.status)
{
    this->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 10px; margin: 10px;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 1. 헤더
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *textLayout = new QVBoxLayout();

    QLabel *lblStore = new QLabel(QString::fromStdString(info.storeName));
    lblStore->setStyleSheet("font-size: 18px; font-weight: bold; color: #333333; border: none;");

    QLabel *lblDate = new QLabel(QString::fromStdString(info.createdAt));
    lblDate->setStyleSheet("font-size: 13px; color: #999999; border: none;");

    textLayout->addWidget(lblStore);
    textLayout->addWidget(lblDate);
    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();

    QLabel *lblImage = new QLabel();
    lblImage->setFixedSize(70, 70);
    lblImage->setStyleSheet("background-color: #f5f5f5; border-radius: 8px; border: 1px solid #eeeeee;");
    headerLayout->addWidget(lblImage);
    mainLayout->addLayout(headerLayout);

    // 2. 상태 표시 ← 수정
    QString statusText;
    if (info.status == 9) {
        statusText = "배달 거절";
    } else {
        statusText = "배달 완료";
    }
    QLabel *lblStatus = new QLabel(statusText);
    lblStatus->setStyleSheet(info.status == 9
                                 ? "font-size: 15px; font-weight: bold; color: #e53935; border: none; margin-bottom: 5px;"
                                 : "font-size: 15px; font-weight: bold; color: #555555; border: none; margin-bottom: 5px;");
    mainLayout->addWidget(lblStatus);

    // 3. 메뉴 요약
    QLabel *lblMenuSummary = new QLabel(QString::fromStdString(info.menuSummary));
    lblMenuSummary->setStyleSheet("font-size: 15px; color: #444444; border: none; margin-left: 5px;");
    mainLayout->addWidget(lblMenuSummary);

    // 4. 총 금액
    QLabel *lblTotal = new QLabel("합계: " + StoreUtils::formatWon(info.totalPrice));
    lblTotal->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111; border: none; margin-top: 10px;");
    mainLayout->addWidget(lblTotal);

    // 5. 버튼 — 거절이면 숨김 ← 수정
    if (info.status != 9) {
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(10);
        btnLayout->setContentsMargins(0, 15, 0, 0);

        QPushButton *btnReorder = new QPushButton("재주문하기");
        btnReorder->setFixedHeight(45);
        btnReorder->setStyleSheet("background-color: #00BAF6; color: white; font-weight: bold; border-radius: 6px; border: none;");

        QPushButton *btnReview = new QPushButton("리뷰 쓰기");
        btnReview->setFixedHeight(45);
        btnReview->setStyleSheet("background-color: white; color: #00BAF6; border: 1px solid #00BAF6; font-weight: bold; border-radius: 6px;");

        btnLayout->addWidget(btnReorder);
        btnLayout->addWidget(btnReview);
        mainLayout->addLayout(btnLayout);
    }
}

void OrderHistoryCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_status != 9) { // ← 조건 추가
        emit receiptRequested(m_orderId);
    }
    QFrame::mousePressEvent(event);
}