#include "form.h"
#include "ui_form.h"
#include "storeutils.h"
// #include <QLocale> // 맨 위에 없다면 추가해 주세요 (천 단위 콤마용)

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    // 1. 초기 상태 설정
    ui->statusStackedWidget->setCurrentIndex(0);
    // (삭제됨) ui->menuDetailContainer->hide(); 

    // 2. 🚀 X 버튼 로직
    connect(ui->btn_back_form, &QPushButton::clicked, this, [=]() {
        emit backRequested(); // "이 화면 닫을래!"라고 신호 보냄
    });

    // 3. 메뉴 확장 버튼 (상자가 삭제되었으므로 일단 비워둡니다)
    connect(ui->btn_expand_menu, &QPushButton::clicked, this, [=]() {
        // 나중에 상자를 다시 만들면 여기에 코드를 넣습니다.
        //bool isHidden = ui->menuDetailContainer->isHidden();
        //ui->menuDetailContainer->setVisible(isHidden);
        //ui->btn_expand_menu->setText(isHidden ? "▲" : "▼");
    });
}

Form::~Form()
{
    delete ui;
}

// 🚀 실시간 배달 상태 업데이트 (MainWindow에서 호출됨)
void Form::updateStatus(int index) {
    // 0:접수, 1:조리중, 2:배달중, 3:배달완료
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

void Form::clearMenuItems() {
    if (!ui->menuItemsArea) return; // 방어 코드

    QLayoutItem *child;
    // menuItemsArea 레이아웃 안에 있는 예전 글씨들을 깨끗하게 청소합니다.
    while ((child = ui->menuItemsArea->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

// 영수증에 메뉴 한 줄 추가하기 (진짜 구현)
void Form::addMenuItem(const QString &menuName, int quantity, int price) {
    if (!ui->menuItemsArea) return;

    // 1. 가로로 배치할 투명한 빈 박스 만들기
    QWidget *itemWidget = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(itemWidget);
    hLayout->setContentsMargins(10, 5, 10, 5); // 위아래 여백

    // 2. 왼쪽 글자 (예: 짜장면 x1)
    QString nameText = QString("%1 x%2").arg(menuName).arg(quantity);
    QLabel *nameLabel = new QLabel(nameText, itemWidget);
    nameLabel->setStyleSheet("color: #333333; font-size: 14px;");

    // 3. 오른쪽 글자 (예: 7,000원) - QLocale로 콤마(,) 찍기
    QString priceText = StoreUtils::formatWon(price);
    QLabel *priceLabel = new QLabel(priceText, itemWidget);
    priceLabel->setStyleSheet("color: #111111; font-size: 14px; font-weight: bold;");
    priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 4. 박스에 조립하기 (왼쪽 글자 - 빈공간 쫙 늘리기 - 오른쪽 글자)
    hLayout->addWidget(nameLabel);
    hLayout->addStretch(); 
    hLayout->addWidget(priceLabel);

    // 5. 드디어 화면(menuItemsArea)에 추가!
    ui->menuItemsArea->addWidget(itemWidget);
}