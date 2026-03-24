#include "cartbarwidget.h"
#include "ui_cartbarwidget.h"
#include <QLocale>

CartBarWidget::CartBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CartBarWidget)
{
    ui->setupUi(this);

    // 카트 보기 버튼을 누르면 밖으로 cartRequested 신호를 발사함
    connect(ui->btnCartView, &QPushButton::clicked, this, &CartBarWidget::cartRequested);

    // 이 위젯이 처음 화면에 나타날 때 현재 장바구니 상태에 맞춰서 그려줌
    updateCartUI();
}

CartBarWidget::~CartBarWidget() { delete ui; }

// ============================================================
// 스스로 장바구니 상태를 확인하고 화면을 바꾸는 핵심 기능!
// ============================================================
void CartBarWidget::updateCartUI()
{
    int count = CartSession::instance().totalCount();
    int price = CartSession::instance().totalPrice();

    if (count > 0) {
        this->show(); // 장바구니에 1개라도 있으면 자기 자신을 화면에 보여줌
        ui->btnCartView->setText(QString("  %1   카트 보기").arg(count));

        // 가격에 천 단위 콤마(,) 찍어주기
        QLocale locale(QLocale::Korean);
        ui->labelCartPrice->setText(locale.toString(price) + "원");
    } else {
        this->hide(); // 비었으면 알아서 화면에서 숨음
    }
}