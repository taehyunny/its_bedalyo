#pragma once

#include <QWidget>
#include <QPainter>
#include "cartsession.h"

namespace Ui { class CartBarWidget; }

class CartBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartBarWidget(QWidget *parent = nullptr);
    ~CartBarWidget();

    void updateCartUI();

signals:
    void cartRequested();

protected:
    void paintEvent(QPaintEvent *e) override; // 배경색 직접 그리기

private:
    Ui::CartBarWidget *ui;
};
