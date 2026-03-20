#include "HomeWidget.h"
#include "ui_homewidget.h"

HomeWidget::HomeWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomeWidget)
    , m_network(network)
{
    ui->setupUi(this);

    connect(ui->btnCatAll,      &QPushButton::clicked, this, [this]{ onCategoryClicked("전체"); });
    connect(ui->btnCatChicken,  &QPushButton::clicked, this, [this]{ onCategoryClicked("치킨"); });
    connect(ui->btnCatKorean,   &QPushButton::clicked, this, [this]{ onCategoryClicked("한식"); });
    connect(ui->btnCatChinese,  &QPushButton::clicked, this, [this]{ onCategoryClicked("중식"); });
    connect(ui->btnCatJapanese, &QPushButton::clicked, this, [this]{ onCategoryClicked("일식"); });
    connect(ui->btnCatWestern,  &QPushButton::clicked, this, [this]{ onCategoryClicked("양식"); });
    connect(ui->btnCatCafe,     &QPushButton::clicked, this, [this]{ onCategoryClicked("카페"); });
    connect(ui->btnCatBakery,   &QPushButton::clicked, this, [this]{ onCategoryClicked("베이커리"); });

    // TODO: NetworkManager 시그널 연결
    // connect(m_network, &NetworkManager::onStoreListReceived,
    //         this, &HomeWidget::populateStoreList);

    // 초기 목록 요청
    requestStoreList("전체");
}

HomeWidget::~HomeWidget() { delete ui; }

void HomeWidget::setUserName(const QString &userName) { m_userName = userName; }

// ============================================================
// 서버에 가게 목록 요청
// TODO: NetworkManager에 sendStoreListRequest() 구현 후 연결
// ============================================================
void HomeWidget::requestStoreList(const QString &category)
{
    Q_UNUSED(category)
    // m_network->sendStoreListRequest(category);

    // 서버 연결 전 — 빈 목록으로 초기화만
    ui->storeList->clear();
}

// ============================================================
// 서버 응답 수신 후 목록 채우기
// NetworkManager 시그널 연결 시 슬롯으로 사용
// ============================================================
void HomeWidget::populateStoreList(const QList<StoreInfo> &stores)
{
    ui->storeList->clear();

    for (const StoreInfo &store : stores) {
        StoreItemWidget *itemWidget = new StoreItemWidget();
        itemWidget->setData(
            store.name, store.category,
            store.rating, store.reviewCount,
            store.distanceKm,
            store.minDeliveryTime, store.maxDeliveryTime,
            store.deliveryFee, store.minOrderAmount,
            store.tags
        );

        QListWidgetItem *item = new QListWidgetItem(ui->storeList);
        item->setData(Qt::UserRole, store.storeId);
        item->setSizeHint(QSize(390, StoreItemWidget::itemHeight()));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setBackground(Qt::transparent);

        ui->storeList->addItem(item);
        ui->storeList->setItemWidget(item, itemWidget);
    }
}

// ============================================================
// 카테고리 선택
// ============================================================
void HomeWidget::onCategoryClicked(const QString &category)
{
    m_currentCategory = category;
    updateCategoryStyle(category);
    requestStoreList(category);
}

void HomeWidget::updateCategoryStyle(const QString &selectedCategory)
{
    const QString ACTIVE   = "background-color:#1565c0; color:#ffffff; border:1.5px solid #1565c0; border-radius:20px; font-size:12px; font-weight:bold; padding:6px 14px; min-height:34px;";
    const QString INACTIVE = "background-color:#ffffff; color:#444444; border:1.5px solid #e8e8e8; border-radius:20px; font-size:12px; font-weight:bold; padding:6px 14px; min-height:34px;";

    QMap<QPushButton*, QString> catMap = {
        { ui->btnCatAll,      "전체"      }, { ui->btnCatChicken,  "치킨"      },
        { ui->btnCatKorean,   "한식"      }, { ui->btnCatChinese,  "중식"      },
        { ui->btnCatJapanese, "일식"      }, { ui->btnCatWestern,  "양식"      },
        { ui->btnCatCafe,     "카페"      }, { ui->btnCatBakery,   "베이커리"  },
    };
    for (auto it = catMap.begin(); it != catMap.end(); ++it)
        it.key()->setStyleSheet(it.value() == selectedCategory ? ACTIVE : INACTIVE);
}

void HomeWidget::on_storeList_itemClicked(QListWidgetItem *item)
{
    emit storeSelected(item->data(Qt::UserRole).toInt());
}

void HomeWidget::on_navHome_clicked()   {}
void HomeWidget::on_navSearch_clicked() { emit searchRequested(); }
void HomeWidget::on_navOrder_clicked()  { emit orderListRequested(); }
void HomeWidget::on_navMypage_clicked() { emit mypageRequested(); }
