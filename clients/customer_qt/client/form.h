#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

public slots:
    // 동적 구현을 위한 슬롯들
    void updateStatus(int index);
    void updateOrderInfo(const QString &storeName, const QString &orderNo, const QString &menuSummary);
    void updateAddress(const QString &address);

private:
    Ui::Form *ui;
};

#endif // FORM_H