#pragma once

#include <QWidget>
#include "NetworkManager.h"

namespace Ui { class OrderCompleteWidget; }

class OrderCompleteWidget : public QWidget {
    Q_OBJECT

public:
    explicit OrderCompleteWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~OrderCompleteWidget();

    // 🚀 [추가] 밖(MainWindow)에서 동적 데이터를 밀어넣어 줄 함수들!
    void setOrderData(const QString& orderId, const QString& storeName, const QString& address);
    void addMenuItem(int quantity, const QString& menuName);
    void clearMenuItems(); // 이전 메뉴 내역 지우기

private:
    Ui::OrderCompleteWidget *ui;
    NetworkManager *m_network;
};