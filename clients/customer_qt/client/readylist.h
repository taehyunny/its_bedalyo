#ifndef READYLIST_H
#define READYLIST_H
#include <QMap> // 추가
#include <QLabel> // 추가

#include <QWidget>

namespace Ui {
class readylist;
}

class readylist : public QWidget
{
    Q_OBJECT

public:
    explicit readylist(QWidget *parent = nullptr);
    ~readylist();

signals:
    void orderDetailRequested(const QString &orderId); // 상세보기가 눌렸음을 알리는 시그널
    void allCardsRemoved();

public slots:
    // 가게이름, 상태, 메뉴목록(문자열), 합계금액을 인자로 받음
    void addOrderCard(const QString &orderId, const QString &storeName, const QString &status,
                      const QString &menuList, const QString &totalPrice);
    
    
    void updateCardStatus(const QString &orderId, int state);
    void removeOrderCard(const QString &orderId);

private:
    Ui::readylist *ui;
    QMap<QString, QLabel*> m_statusLabels;
};

#endif // READYLIST_H