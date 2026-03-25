#ifndef READYLIST_H
#define READYLIST_H

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

public slots:
    // 가게이름, 상태, 메뉴목록(문자열), 합계금액을 인자로 받음
    void addOrderCard(const QString &storeName, const QString &status,
                      const QString &menuList, const QString &totalPrice);

private:
    Ui::readylist *ui;
};

#endif // READYLIST_H