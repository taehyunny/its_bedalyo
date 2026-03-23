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

    // 뒤로가기 버튼 연결
    connect(ui->btnBack, &QPushButton::clicked, this, &StoreDetailWidget::on_btnBack_clicked);

    // 🚀 [수정됨] 이제 onMenuListReceived가 아니라 onStoreDetailReceived를 듣습니다!
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

    // 2. 서버에 진짜로 상세 정보(메뉴+리뷰+가게) 요청 발사!
    m_network->sendStoreDetailRequest(storeId);
}

// ============================================================
// 🚀 [수정됨] 서버에서 '종합 선물 세트'를 받았을 때 실행됨!
// ============================================================
void StoreDetailWidget::onStoreDetailReceived(StoreDetailQt detail)
{
    if (detail.storeId != m_currentStoreId) return;

    // 1. 가게 이름 세팅
    ui->lblStoreName->setText(detail.storeName);

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