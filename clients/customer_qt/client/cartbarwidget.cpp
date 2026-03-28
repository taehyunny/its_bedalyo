#include "cartbarwidget.h"
#include "ui_cartbarwidget.h"
#include <QLocale>
#include <QPainter>

CartBarWidget::CartBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CartBarWidget)
{
    ui->setupUi(this);
    connect(ui->btnCartView, &QPushButton::clicked, this, &CartBarWidget::cartRequested);
    updateCartUI();
}

CartBarWidget::~CartBarWidget() { delete ui; }

// ─── 배경색을 직접 그려야 QWidget에서 제대로 표시됨 ───
void CartBarWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#1565c0"));
    QWidget::paintEvent(e);
}

void CartBarWidget::updateCartUI()
{
    int count = CartSession::instance().totalCount();
    int price = CartSession::instance().totalPrice();

    if (count > 0) {
        ui->btnCartView->setText(
            QString("  %1   카트 보기").arg(count));

        QLocale locale(QLocale::Korean);
        ui->labelCartPrice->setText(StoreUtils::formatWon(price));
        ui->labelCartSubtext->setText(QString("담은 메뉴 %1개").arg(count));

        this->show();
    } else {
        this->hide();
    }
}
