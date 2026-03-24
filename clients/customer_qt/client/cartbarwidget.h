#pragma once

#include <QWidget>
#include "cartsession.h" // 장바구니 데이터를 읽어오기 위해 꼭 필요해!

namespace Ui { class CartBarWidget; }

class CartBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartBarWidget(QWidget *parent = nullptr);
    ~CartBarWidget();

    // 자기 스스로 UI(글씨, 가격)를 새로고침하는 기능!
    void updateCartUI();

signals:
    // 유저가 카트 보기를 눌렀을 때 밖으로 "나 눌렸어!" 하고 외치는 신호
    void cartRequested();

private:
    Ui::CartBarWidget *ui;
};