#include "HomeWidget.h"
#include "ui_homewidget.h"
#include <QFrame>
#include <QLocale>
#include <QScrollBar>
#include <QPainter>

// ============================================================
// DragScrollArea
// ============================================================
DragScrollArea::DragScrollArea(QWidget *parent) : QScrollArea(parent) {}

void DragScrollArea::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos  = e->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QScrollArea::mousePressEvent(e);
}

void DragScrollArea::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) {
        int delta = e->pos().x() - m_lastPos.x();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta);
        m_lastPos = e->pos();
    }
    QScrollArea::mouseMoveEvent(e);
}

void DragScrollArea::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
    QScrollArea::mouseReleaseEvent(e);
}

// ============================================================
// HomeWidget
// ============================================================
HomeWidget::HomeWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomeWidget)
    , m_network(network)
{
    ui->setupUi(this);

    connect(ui->navHome,     &QPushButton::clicked, this, &HomeWidget::on_navHome_clicked);
    connect(ui->navSearch,   &QPushButton::clicked, this, &HomeWidget::on_navSearch_clicked);
    connect(ui->navFavorite, &QPushButton::clicked, this, &HomeWidget::on_navFavorite_clicked);
    connect(ui->navOrder,    &QPushButton::clicked, this, &HomeWidget::on_navOrder_clicked);
    connect(ui->navMy,       &QPushButton::clicked, this, &HomeWidget::on_navMy_clicked);
    connect(ui->btnSearch,   &QPushButton::clicked, this, &HomeWidget::on_btnSearch_clicked);
}

HomeWidget::~HomeWidget() { delete ui; }

void HomeWidget::setUserName(const QString &userName) { m_userName = userName; }
void HomeWidget::setAddress(const QString &address)
{
    m_address = address;
    ui->label_address->setText(address.isEmpty() ? "주소 없음" : address);
}

void HomeWidget::populateCategories1(const QList<CategoryInfo> &categories)
{
    QLayout *layout = ui->catContent1->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const CategoryInfo &cat : categories)
        layout->addWidget(makeCategoryItem(cat));
    static_cast<QHBoxLayout*>(layout)->addStretch();
}

void HomeWidget::populateCategories2(const QList<CategoryInfo> &categories)
{
    QLayout *layout = ui->catContent2->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const CategoryInfo &cat : categories)
        layout->addWidget(makeCategoryItem(cat));
    static_cast<QHBoxLayout*>(layout)->addStretch();
}

void HomeWidget::populateStoreList(const QList<StoreInfo> &stores)
{
    QLayout *layout = ui->storeContent->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const StoreInfo &store : stores)
        layout->addWidget(makeStoreCard(store));
    static_cast<QVBoxLayout*>(layout)->addStretch();
}

QWidget* HomeWidget::makeCategoryItem(const CategoryInfo &cat)
{
    QWidget *item = new QWidget();
    item->setFixedWidth(60);
    item->setStyleSheet("background: transparent;");

    QVBoxLayout *vl = new QVBoxLayout(item);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(6);
    vl->setAlignment(Qt::AlignHCenter);

    QPushButton *imgBtn = new QPushButton();
    imgBtn->setFixedSize(52, 52);
    imgBtn->setStyleSheet(
        "QPushButton { background:#f0f4ff; border-radius:26px; border:none; font-size:24px; }"
        "QPushButton:hover { background:#dce8ff; }"
    );
    // TODO: 서버에서 icon_name(예: korean_food.png) 받아온 후 아래 주석 해제
    // QPixmap px; px.loadFromData(imageBytes); imgBtn->setIcon(QIcon(px)); imgBtn->setText("");
    imgBtn->setText("🍽"); // 임시 placeholder — DB의 icon_name 기반 실제 이미지로 교체 필요

    QLabel *nameLabel = new QLabel(cat.name.isEmpty() ? "-" : cat.name);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size:11px; color:#333333; background:transparent;");

    vl->addWidget(imgBtn, 0, Qt::AlignHCenter);
    vl->addWidget(nameLabel, 0, Qt::AlignHCenter);

    int catId = cat.categoryId;
    QString catName = cat.name;
    connect(imgBtn, &QPushButton::clicked, this, [this, catId, catName]() {
        emit categorySelected(catId, catName);
    });

    return item;
}

QWidget* HomeWidget::makeStoreCard(const StoreInfo &store)
{
    QWidget *card = new QWidget();
    card->setStyleSheet("background:#ffffff;");
    card->setCursor(Qt::PointingHandCursor);
    card->setProperty("storeId", store.storeId);
    card->installEventFilter(this);

    QVBoxLayout *vl = new QVBoxLayout(card);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    QLabel *imgLabel = new QLabel();
    imgLabel->setFixedHeight(200);
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setStyleSheet(
        QString("background-color:%1; font-size:48px;")
            .arg(placeholderColorForCard(store.category))
    );
    // TODO: store.imageUrl 로 서버에서 이미지 받아온 후 아래 주석 해제
    // QNetworkAccessManager로 imageUrl 요청 → QPixmap 변환 → imgLabel->setPixmap(pixmap)
    // imgLabel->setScaledContents(true);
    imgLabel->setText(categoryEmoji(store.category)); // 임시 — 실제 가게 사진으로 교체 필요
    vl->addWidget(imgLabel);

    QWidget *info = new QWidget();
    info->setStyleSheet("background:#ffffff;");
    QVBoxLayout *il = new QVBoxLayout(info);
    il->setContentsMargins(14, 10, 14, 12);
    il->setSpacing(4);

    QLabel *catLabel = new QLabel(store.category);
    catLabel->setStyleSheet(
        "font-size:11px; color:#1565c0; background:#e8f0ff;"
        "border-radius:4px; padding:2px 8px; font-weight:bold;");
    catLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    QLabel *nameLabel = new QLabel(store.name.isEmpty() ? "-" : store.name);
    nameLabel->setStyleSheet("font-size:17px; font-weight:bold; color:#111111;");

    QLabel *metaLabel = new QLabel(
        QString("⭐ %1 (%2)  ·  %3  ·  %4")
            .arg(store.rating, 0, 'f', 1).arg(store.reviewCount)
            .arg(formatDeliveryFee(store.deliveryFee))
            .arg(formatTime(store.minDeliveryTime, store.maxDeliveryTime))
    );
    metaLabel->setStyleSheet("font-size:13px; color:#555555;");

    QLabel *minOrderLabel = new QLabel("최소주문 " + formatWon(store.minOrderAmount));
    minOrderLabel->setStyleSheet("font-size:12px; color:#888888;");

    il->addWidget(catLabel);
    il->addWidget(nameLabel);
    il->addWidget(metaLabel);
    il->addWidget(minOrderLabel);
    vl->addWidget(info);

    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color:#f0f0f0;");
    divider->setFixedHeight(1);
    vl->addWidget(divider);

    return card;
}

bool HomeWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (w) {
            QVariant v = w->property("storeId");
            if (v.isValid()) emit storeSelected(v.toInt());
        }
    }
    return QWidget::eventFilter(obj, event);
}

QString HomeWidget::formatWon(int amount)
    { return QLocale(QLocale::Korean).toString(amount) + "원"; }
QString HomeWidget::formatDeliveryFee(int fee)
    { return (fee == 0) ? "무료배달" : "배달비 " + QLocale(QLocale::Korean).toString(fee) + "원"; }
QString HomeWidget::formatTime(int min, int max)
    { return (max > 0) ? QString("%1~%2분").arg(min).arg(max) : QString("%1분").arg(min); }
// TODO: 실제 이미지 적용 후 이 함수는 불필요해짐 (이미지 로드 전 잠깐 보이는 배경색)
QString HomeWidget::placeholderColorForCard(const QString &cat)
{
    if (cat == "중식") return "#fde8d8"; if (cat == "일식") return "#d8eafd";
    if (cat == "치킨") return "#fdf5d8"; if (cat == "한식") return "#d8fde4";
    if (cat == "양식") return "#fdd8d8"; return "#f0f0f0";
}
// TODO: 실제 이미지 적용 후 이 함수는 불필요해짐
QString HomeWidget::categoryEmoji(const QString &cat)
{
    if (cat == "중식") return "🥟"; if (cat == "일식") return "🍱";
    if (cat == "치킨") return "🍗"; if (cat == "한식") return "🍚";
    if (cat == "양식") return "🍕"; return "🍽";
}

void HomeWidget::on_btnSearch_clicked()    { emit searchRequested(); }
void HomeWidget::on_navHome_clicked()      {}
void HomeWidget::on_navSearch_clicked()    { emit searchRequested(); }
void HomeWidget::on_navFavorite_clicked()  { emit favoriteRequested(); }
void HomeWidget::on_navOrder_clicked()     { emit orderListRequested(); }
void HomeWidget::on_navMy_clicked()        { emit mypageRequested(); }
