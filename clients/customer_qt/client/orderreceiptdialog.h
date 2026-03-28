#pragma once
#include <QDialog>
#include "PaymentDTO.h"

class OrderReceiptDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderReceiptDialog(const ResOrderDetailDTO& data, QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
};
