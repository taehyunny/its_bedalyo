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
    
    void setCurrentOrderId(const QString &id) { m_currentOrderId = id; }
    QString currentOrderId() const { return m_currentOrderId; }

signals:
    void backRequested();

public slots:
    // 동적 구현을 위한 슬롯들
    void updateStatus(int index);
    void updateOrderInfo(const QString &storeName, const QString &orderNo, const QString &menuSummary);
    void updateAddress(const QString &address);
    void clearMenuItems(); 
    void addMenuItem(const QString &menuName, int quantity, int price);

private:
    Ui::Form *ui;
    QString m_currentOrderId;
};

#endif // FORM_H