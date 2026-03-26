#include "OrderHistoryCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLocale>
#include <QMouseEvent>

OrderHistoryCard::OrderHistoryCard(const OrderHistoryItemDTO& info, QWidget* parent)
    : QFrame(parent)
    , m_orderId(QString::fromStdString(info.orderId)) // 생성될 때 주문번호 저장
{
    this->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 10px; margin: 10px;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 1. 헤더 (가게명 + 날짜 + 이미지)
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *textLayout = new QVBoxLayout();
    
    QLabel *lblStore = new QLabel(QString::fromStdString(info.storeName));
    lblStore->setStyleSheet("font-size: 18px; font-weight: bold; color: #333333; border: none;");
    
    QLabel *lblDate = new QLabel(QString::fromStdString(info.createdAt)); // DB의 날짜
    lblDate->setStyleSheet("font-size: 13px; color: #999999; border: none;");
    
    textLayout->addWidget(lblStore);
    textLayout->addWidget(lblDate);
    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();
    
    // 사진 자리 (나중에 info.deliveryPhotoUrl 활용 가능)
    QLabel *lblImage = new QLabel();
    lblImage->setFixedSize(70, 70);
    lblImage->setStyleSheet("background-color: #f5f5f5; border-radius: 8px; border: 1px solid #eeeeee;");
    headerLayout->addWidget(lblImage);
    
    mainLayout->addLayout(headerLayout);

    // 2. 상태 표시
    QLabel *lblStatus = new QLabel("배달 완료");
    lblStatus->setStyleSheet("font-size: 15px; font-weight: bold; color: #555555; border: none; margin-bottom: 5px;");
    mainLayout->addWidget(lblStatus);

    // 3. 메뉴 요약 (복잡한 for문 삭제! 팀장님의 menuSummary 활용)
    QLabel *lblMenuSummary = new QLabel(QString::fromStdString(info.menuSummary));
    lblMenuSummary->setStyleSheet("font-size: 15px; color: #444444; border: none; margin-left: 5px;");
    mainLayout->addWidget(lblMenuSummary);

    // 4. 총 금액
    QLabel *lblTotal = new QLabel(QString("합계: %1원").arg(QLocale(QLocale::Korean).toString(info.totalPrice)));
    lblTotal->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111; border: none; margin-top: 10px;");
    mainLayout->addWidget(lblTotal);

    // 5. 버튼 (재주문 / 리뷰쓰기)
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

void OrderHistoryCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit receiptRequested(m_orderId); // 클릭되면 시그널 발사!
    }
    QFrame::mousePressEvent(event);
}