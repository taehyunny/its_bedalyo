#ifndef DELIVERYCOMPLETEWIDGET_H
#define DELIVERYCOMPLETEWIDGET_H

#include <QWidget>
#include "NetworkManager.h" // 통신 관리자 포함!

namespace Ui {
class DeliveryCompleteWidget;
}

class DeliveryCompleteWidget : public QWidget
{
    Q_OBJECT

public:

    void setOrderId(const QString &orderId) { m_orderId = orderId; }

    // 메인 프로젝트 환경에 맞게 NetworkManager를 받도록 설정
    explicit DeliveryCompleteWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~DeliveryCompleteWidget();

private:
    Ui::DeliveryCompleteWidget *ui;
    NetworkManager *m_network; // 넘겨받은 통신 관리자를 저장할 변수

    int m_rating; // 별점 점수를 기억할 변수

    // 별 모양을 업데이트하는 함수
    void updateStars(int rating);

    QString m_orderId;

signals:
    void orderListRequested();
    void orderCompleted(const QString &orderId);
};

#endif // DELIVERYCOMPLETEWIDGET_H