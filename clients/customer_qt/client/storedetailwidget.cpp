#include "storedetailwidget.h"
#include "ui_storedetailwidget.h"
#include "storeutils.h"
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
// 🚀 [수정됨] 매개변수가 MenuDTO에서 MenuQt로 바뀌었습니다!
QWidget* StoreDetailWidget::makeMenuCard(const MenuQt& menu) 
{
    QWidget* card = new QWidget();
    card->setStyleSheet("QWidget { background-color: #ffffff; border-bottom: 1px solid #eeeeee; }");
    card->setMinimumHeight(100);

    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    // 🚀 [수정됨] QString::fromStdString(...) 부분이 없어지고 그냥 menu.menuName을 씁니다.
    QLabel* nameLabel = new QLabel(menu.menuName);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111; border: none;");

    QLabel* descLabel = new QLabel(menu.description);
    descLabel->setStyleSheet("font-size: 13px; color: #888888; border: none;");
    descLabel->setWordWrap(true);

    QLabel* priceLabel = new QLabel(StoreUtils::formatWon(menu.basePrice));
    priceLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #111111; border: none;");

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descLabel);
    textLayout->addWidget(priceLabel);
    textLayout->addStretch();

    mainLayout->addLayout(textLayout);

    QLabel* imgLabel = new QLabel();
    imgLabel->setFixedSize(80, 80);
    imgLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 8px; border: none;");
    mainLayout->addWidget(imgLabel);

    QPushButton *clickOverlay = new QPushButton(card);
    clickOverlay->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: rgba(0,0,0,0.03); }"
        "QPushButton:pressed { background: rgba(0,0,0,0.07); }"
        );
    clickOverlay->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* overlayLayout = new QVBoxLayout(card);
    overlayLayout->setContentsMargins(0,0,0,0);
    overlayLayout->addWidget(clickOverlay);
    clickOverlay->raise();

    connect(clickOverlay, &QPushButton::clicked, this, [this, menu]() {
        if (menu.isSoldOut) {
            qDebug() << "품절된 메뉴입니다:" << menu.menuName;
            return;
        }
        // 🚀 [수정됨] 여기도 그냥 menu.menuName 사용
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