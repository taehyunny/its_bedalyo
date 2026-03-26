#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    // 1. 초기 상태: 가게접수 대기(0번) 및 메뉴 상세 닫힘
    ui->statusStackedWidget->setCurrentIndex(0);
    ui->menuDetailContainer->hide();

    // 2. 뒤로가기 버튼: MainWindow에서 이 시그널을 받아 화면을 전환하게 됩니다.
    connect(ui->btn_back_form, &QPushButton::clicked, this, [=]() {
        // MainWindow에서 '이전 화면으로 돌아가기' 처리를 하도록 시그널을 보내거나 
        // 직접 부모 위젯(StackedWidget)의 인덱스를 조정할 수 있습니다.
        this->hide(); 
    });

    // 3. 메뉴 상세 펼치기/접기 (아코디언)
    connect(ui->btn_expand_menu, &QPushButton::clicked, this, [=]() {
        bool isHidden = ui->menuDetailContainer->isHidden();
        ui->menuDetailContainer->setVisible(isHidden);
        ui->btn_expand_menu->setText(isHidden ? "▲" : "▼");
    });
}

Form::~Form()
{
    delete ui;
}

// 🚀 실시간 배달 상태 업데이트 (MainWindow에서 호출됨)
void Form::updateStatus(int index) {
    // 0:가게접수, 1:조리중, 2:조리완료, 3:배달중
    if (index >= 0 && index < ui->statusStackedWidget->count()) {
        ui->statusStackedWidget->setCurrentIndex(index);
    }
}

// 주문 기본 정보 업데이트
void Form::updateOrderInfo(const QString &storeName, const QString &orderNo, const QString &menuSummary) {
    ui->lbl_store_name->setText(storeName);
    ui->lbl_order_summary->setText(QString("주문번호 %1 | %2").arg(orderNo).arg(menuSummary));
}

// 배달 주소 업데이트
void Form::updateAddress(const QString &address) {
    ui->lbl_address_content->setText(address);
}