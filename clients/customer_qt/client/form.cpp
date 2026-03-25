#include "form.h"
#include "ui_form.h"
#include <QTimer>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    // 초기 상태 설정
    ui->statusStackedWidget->setCurrentIndex(0);
    ui->menuDetailContainer->hide();

    // 메뉴 확장 버튼 연결
    connect(ui->btn_expand_menu, &QPushButton::clicked, this, [=]() {
        if (ui->menuDetailContainer->isHidden()) {
            ui->menuDetailContainer->show();
            ui->btn_expand_menu->setText("▲");
        } else {
            ui->menuDetailContainer->hide();
            ui->btn_expand_menu->setText("▼");
        }
    });
}

Form::~Form()
{
    delete ui;
}

void Form::updateStatus(int index) {
    ui->statusStackedWidget->setCurrentIndex(index);
}

void Form::updateOrderInfo(const QString &storeName, const QString &orderNo, const QString &menuSummary) {
    ui->lbl_store_name->setText(storeName);
    ui->lbl_order_summary->setText(QString("주문번호 %1 | %2").arg(orderNo).arg(menuSummary));
}

void Form::updateAddress(const QString &address) {
    ui->lbl_address_content->setText(address);
}