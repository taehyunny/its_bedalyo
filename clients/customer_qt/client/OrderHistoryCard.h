#pragma once
#include <QFrame>
#include <QString>
#include <QList>
#include <QMouseEvent>
#include "OrderDTO.h" // OrderItemDTO 사용을 위해 포함

// 카드에 전달할 데이터 묶음
struct PastOrderInfo {
    QString orderId;
    QString storeName;
    QString orderDate;
    int totalPrice;
    QList<OrderItemDTO> items;
};

// 독립된 카드 위젯 클래스
class OrderHistoryCard : public QFrame {
    Q_OBJECT
public:
    // DTO를 받도록 수정
    explicit OrderHistoryCard(const OrderHistoryItemDTO& info, QWidget* parent = nullptr);
    QString getOrderId() const { return m_orderId; }

signals:
    void receiptRequested(const QString& orderId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString m_orderId; // 주문번호를 기억할 변수 추가
};