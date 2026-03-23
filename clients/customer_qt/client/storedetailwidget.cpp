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

    // 🚀 [중요] 나중에 NetworkManager에 이 시그널을 만들면 주석을 푸세요!
    // connect(m_network, &NetworkManager::onMenuListReceived,
    //         this, &StoreDetailWidget::onMenuListReceived);
}

StoreDetailWidget::~StoreDetailWidget() { delete ui; }

// ============================================================
// 진입점: MainWindow에서 넘어올 때 무조건 실행됨!
// ============================================================
void StoreDetailWidget::loadStoreData(int storeId)
{
    m_currentStoreId = storeId;
    qDebug() << "[StoreDetailWidget] 가게 상세 진입! 요청 ID:" << storeId;

    // 1. 기존 메뉴들 청소
    clearLayout(ui->menuListLayout);
    
    ui->lblStoreName->setText("메뉴를 불러오는 중...");

    // 2. 서버에 메뉴 요청! (NetworkManager에 함수 추가 필요)
    // MenuListReqDTO req;
    // req.storeId = storeId;
    // m_network->sendMenuListRequest(req); 
}

// ============================================================
// 서버에서 메뉴 목록을 받았을 때 실행됨
// ============================================================
void StoreDetailWidget::onMenuListReceived(int storeId, QList<MenuDTO> menus)
{
    if (storeId != m_currentStoreId) return;

    ui->lblStoreName->setText("가게 이름 (서버 연동 됨)");

    QMap<QString, QList<MenuDTO>> groupedMenus;
    for (const MenuDTO& menu : menus) {
        QString cat = QString::fromStdString(menu.menuCategory);
        if (cat.isEmpty()) cat = "기본 메뉴";
        groupedMenus[cat].append(menu);
    }

    for (auto it = groupedMenus.begin(); it != groupedMenus.end(); ++it) {
        QLabel* catLabel = new QLabel(it.key());
        catLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#111111; padding:15px 15px 5px 15px;");
        ui->menuListLayout->addWidget(catLabel);

        for (const MenuDTO& menu : it.value()) {
            ui->menuListLayout->addWidget(makeMenuCard(menu));
        }
    }
}

QWidget* StoreDetailWidget::makeMenuCard(const MenuDTO& menu)
{
    // 1. 메뉴 카드 바탕 위젯
    QWidget* card = new QWidget();
    card->setStyleSheet("QWidget { background-color: #ffffff; border-bottom: 1px solid #eeeeee; }");
    card->setMinimumHeight(100);

    // 2. 가로로 배치하기 위한 메인 레이아웃
    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // 3. 텍스트 정보 (왼쪽)
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    // 메뉴 이름 (여기서 menu 변수를 사용하므로 첫 번째 경고 해결!)
    QLabel* nameLabel = new QLabel(QString::fromStdString(menu.menuName));
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111; border: none;");

    // 메뉴 설명
    QLabel* descLabel = new QLabel(QString::fromStdString(menu.description));
    descLabel->setStyleSheet("font-size: 13px; color: #888888; border: none;");
    descLabel->setWordWrap(true);

    // 메뉴 가격 (여기서 StoreUtils를 사용하므로 두 번째 경고 해결!)
    QLabel* priceLabel = new QLabel(StoreUtils::formatWon(menu.basePrice));
    priceLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #111111; border: none;");

    // 텍스트들을 레이아웃에 조립
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descLabel);
    textLayout->addWidget(priceLabel);
    textLayout->addStretch(); // 남은 공간 밀어내기

    mainLayout->addLayout(textLayout);

    // 4. 음식 사진 더미 (오른쪽)
    QLabel* imgLabel = new QLabel();
    imgLabel->setFixedSize(80, 80);
    imgLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 8px; border: none;");
    mainLayout->addWidget(imgLabel);

    // 5. 🚀 카드 전체를 덮는 클릭 버튼 (투명 오버레이)
    QPushButton *clickOverlay = new QPushButton(card);
    clickOverlay->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: rgba(0,0,0,0.03); }"
        "QPushButton:pressed { background: rgba(0,0,0,0.07); }"
        );
    clickOverlay->setCursor(Qt::PointingHandCursor);

    // 오버레이 버튼이 카드를 꽉 채우도록 레이아웃 설정
    QVBoxLayout* overlayLayout = new QVBoxLayout(card);
    overlayLayout->setContentsMargins(0,0,0,0);
    overlayLayout->addWidget(clickOverlay);
    clickOverlay->raise(); // 클릭 버튼을 제일 위로 올리기

    // 6. 클릭 시 동작 설정
    connect(clickOverlay, &QPushButton::clicked, this, [this, menu]() {
        if (menu.isSoldOut) {
            qDebug() << "품절된 메뉴입니다:" << QString::fromStdString(menu.menuName);
            // TODO: 나중에 화면에 "품절입니다" 팝업 띄우기
            return;
        }
        // 4페이지(장바구니/옵션 선택)로 넘기기 위해 MainWindow에 신호 쏘기!
        emit menuSelected(menu.menuId, QString::fromStdString(menu.menuName), menu.basePrice);
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