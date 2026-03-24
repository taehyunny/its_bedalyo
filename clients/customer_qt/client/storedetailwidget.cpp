#include "storedetailwidget.h"
#include "ui_storedetailwidget.h"
#include "storeutils.h"
#include "cartsession.h"
#include <QFrame>
#include <QDebug>
#include <QMap>

StoreDetailWidget::StoreDetailWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StoreDetailWidget)
    , m_network(network)
{
    ui->setupUi(this);
    
    // UI 파일에 만들어둔 메인 뒤로가기 버튼(btnBackToMain) 연결
    connect(ui->btnBackToMain, &QPushButton::clicked, this, &StoreDetailWidget::on_btnBackToMain_clicked);
    
    // 기존 리뷰/정보 페이지용 뒤로가기 버튼 연결
    connect(ui->btnBack, &QPushButton::clicked, this, &StoreDetailWidget::on_btnBack_clicked);
    
    // 메인 뒤로가기 버튼 클릭 시, 기존 뒤로가기 슬롯(on_btnBack_clicked) 실행
    connect(ui->btnBackToMain, &QPushButton::clicked, this, &StoreDetailWidget::on_btnBack_clicked);
    
    // 서버 응답 신호 연결
    connect(m_network, &NetworkManager::onStoreDetailReceived,
            this, &StoreDetailWidget::onStoreDetailReceived);
}

StoreDetailWidget::~StoreDetailWidget() { delete ui; }

// ============================================================
// 진입점: MainWindow에서 넘어올 때 무조건 실행됨!
// ============================================================
void StoreDetailWidget::loadStoreData(int storeId)
{
    // 🚀 [수정됨] 3페이지 진입 시 강제로 0번(메뉴) 화면부터 켜기!
    ui->stackedWidget->setCurrentIndex(0); 

    m_currentStoreId = storeId;
    qDebug() << "[StoreDetailWidget] 가게 상세 진입! 요청 ID:" << storeId;

    // 1. 기존 메뉴들 청소
    clearLayout(ui->menuListLayout);
    ui->lblStoreName->setText("메뉴를 불러오는 중...");

    // 원래 코드: 서버에 요청 (잠시 주석 처리)
    m_network->sendStoreDetailRequest(storeId);

    // 더미 코드: 가짜 데이터 직접 만들어서 UI 함수 강제 실행(테스트용)
    // StoreDetailQt fakeData;
    // fakeData.storeId = storeId;
    // fakeData.storeName = "🔥 테스트용 불향 쭈꾸미 (가짜데이터)";
    
    // MenuQt menu1;
    // menu1.menuName = "쭈꾸미 삼겹살 2인분";
    // menu1.description = "매콤달콤 끝판왕!";
    // menu1.basePrice = 28000;
    // menu1.menuCategory = "메인 메뉴";
    
    // MenuQt menu2;
    // menu2.menuName = "날치알 볶음밥";
    // menu2.description = "K-디저트는 못 참지";
    // menu2.basePrice = 4000;
    // menu2.menuCategory = "사이드 메뉴";
    
    // fakeData.menus.append(menu1);
    // fakeData.menus.append(menu2);

    // // 가짜 데이터를 넣어서 화면 그리는 함수를 직접 호출!
    // onStoreDetailReceived(fakeData);
}

// ============================================================
// 🚀 [수정됨] 서버에서 '종합 선물 세트'를 받았을 때 실행됨!
// ============================================================
void StoreDetailWidget::onStoreDetailReceived(StoreDetailQt detail)
{
    if (detail.storeId != m_currentStoreId) return;

    m_currentStoreName = detail.storeName;

    // 1. 가게 이름 세팅
    ui->lblStoreName->setText(detail.storeName);

    // 별점 & 리뷰수 세팅
    ui->btnRating->setText(QString("⭐ %1 (%2) >").arg(detail.rating, 0, 'f', 1).arg(detail.reviewCount));
    
    // 배달 정보 세팅
    ui->lblDeliveryStats->setText(QString("배달 %1 | 최소주문 %2원 | 배달비 %3")
                                  .arg(detail.deliveryTimeRange).arg(detail.minOrderAmount).arg(detail.deliveryFees));

    // 2. 메뉴판 쫙 깔아주기
    QMap<QString, QList<MenuQt>> groupedMenus;
    for (const MenuQt& menu : detail.menus) {
        QString cat = menu.menuCategory.isEmpty() ? "기본 메뉴" : menu.menuCategory;
        groupedMenus[cat].append(menu);
    }

    for (auto it = groupedMenus.begin(); it != groupedMenus.end(); ++it) {
        QLabel* catLabel = new QLabel(it.key());
        catLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#111111; padding:15px 15px 5px 15px;");
        ui->menuListLayout->addWidget(catLabel);

        for (const MenuQt& menu : it.value()) {
            ui->menuListLayout->addWidget(makeMenuCard(menu));
        }
    }
}

// ============================================================
// 개별 메뉴 카드 만들기
// ============================================================
QWidget* StoreDetailWidget::makeMenuCard(const MenuQt& menu) 
{
    // 1. 껍데기를 QWidget이 아니라 '거대한 버튼(QPushButton)'으로 아예 만듭니다
    QPushButton* card = new QPushButton();
    card->setMinimumHeight(100);
    card->setCursor(Qt::PointingHandCursor);
    
    // 품절 여부에 따른 스타일
    if (menu.isSoldOut) {
        card->setStyleSheet("QPushButton { background-color: #fafafa; border: none; border-bottom: 1px solid #eeeeee; }");
    } else {
        card->setStyleSheet(
            "QPushButton { background-color: #ffffff; border: none; border-bottom: 1px solid #eeeeee; }"
            "QPushButton:pressed { background-color: #f5f5f5; }"
        );
    }

    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    QLabel* nameLabel = new QLabel(menu.menuName);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111;");
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 마우스 클릭 통과

    QLabel* descLabel = new QLabel(menu.description);
    descLabel->setStyleSheet("font-size: 13px; color: #888888;");
    descLabel->setWordWrap(true);
    descLabel->setAttribute(Qt::WA_TransparentForMouseEvents); 

    QString priceText = menu.isSoldOut ? "품절" : StoreUtils::formatWon(menu.basePrice);
    QLabel* priceLabel = new QLabel(priceText);
    priceLabel->setStyleSheet(menu.isSoldOut ? "font-size: 15px; font-weight: bold; color: #d32f2f;" : "font-size: 15px; font-weight: bold; color: #111111;");
    priceLabel->setAttribute(Qt::WA_TransparentForMouseEvents); 

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descLabel);
    textLayout->addWidget(priceLabel);
    textLayout->addStretch();

    mainLayout->addLayout(textLayout);

    QLabel* imgLabel = new QLabel("사진");
    imgLabel->setFixedSize(80, 80);
    imgLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 8px; color:#999;");
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setAttribute(Qt::WA_TransparentForMouseEvents); 
    mainLayout->addWidget(imgLabel);

    // 카드 전체(버튼)를 누르면 신호가 발사되도록 직접 연결!
    connect(card, &QPushButton::clicked, this, [this, menu]() {
        if (menu.isSoldOut) {
            qDebug() << "품절된 메뉴입니다:" << menu.menuName;
            return;
        }
        emit menuSelected(menu.menuId, menu.menuName, menu.basePrice);
    });

    return card;
}

void StoreDetailWidget::on_btnBack_clicked() { emit backRequested(); }

void StoreDetailWidget::clearLayout(QLayout* layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}

void StoreDetailWidget::on_btnStoreInfo_clicked() {
    ui->stackedWidget->setCurrentIndex(1); 
}

void StoreDetailWidget::on_btnStoreInfoBack_clicked() {
    ui->stackedWidget->setCurrentIndex(0); 
}

//  메인 화면에서 뒤로가기 버튼을 눌렀을 때 실행되는 함수
void StoreDetailWidget::on_btnBackToMain_clicked() 
{
    emit backRequested(); // "나 2페이지(가게목록)로 돌아갈래!" 하고 신호 발사!
}

void StoreDetailWidget::updateCartBar()
{
    // CartSession에 담긴 메뉴 개수를 가져옵니다.
    // (CartSession.h가 include 되어 있어야 합니다)
    int count = CartSession::instance().totalCount();

    if (count > 0) {
        // 장바구니에 물건이 있으면 하단 바를 보여줍니다.
        // ui->cartBar는 .ui 파일에 만든 위젯 이름에 맞게 수정하세요.
        // ui->cartBar->show();
        // ui->lblTotalCount->setText(QString::number(count));
        qDebug() << "[StoreDetailWidget] 현재 장바구니 아이템 개수:" << count;
    } else {
        // 비어있으면 숨깁니다.
        // ui->cartBar->hide();
    }
}