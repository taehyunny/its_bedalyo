#include "HomeWidget.h"
#include "ui_homewidget.h"
#include <QFrame>
#include <QLocale>
#include <QScrollBar>
#include <QDebug>

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
// CatScrollFilter — QScrollArea에 드래그 스크롤을 붙이는 이벤트 필터
// ============================================================
class CatScrollFilter : public QObject {
public:
    CatScrollFilter(QScrollArea *target) : QObject(target), m_target(target) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        Q_UNUSED(obj)
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton) {
                m_dragging = true;
                m_lastPos  = e->pos();
                m_target->setCursor(Qt::ClosedHandCursor);
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_dragging) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            int delta = e->pos().x() - m_lastPos.x();
            m_target->horizontalScrollBar()->setValue(
                m_target->horizontalScrollBar()->value() - delta);
            m_lastPos = e->pos();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
            m_target->setCursor(Qt::ArrowCursor);
            return true;
        }
        return false;
    }

private:
    QScrollArea *m_target;
    bool   m_dragging = false;
    QPoint m_lastPos;
};

// ============================================================
// HomeWidget
// ============================================================
HomeWidget::HomeWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomeWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── catScroll1에 드래그 스크롤 필터 설치 ──
    auto *filter1 = new CatScrollFilter(ui->catScroll1);
    ui->catScroll1->viewport()->installEventFilter(filter1);
    ui->catScroll1->viewport()->setMouseTracking(true);

    // ── catScroll2에도 드래그 스크롤 필터 설치 ──
    auto *filter2 = new CatScrollFilter(ui->catScroll2);
    ui->catScroll2->viewport()->installEventFilter(filter2);
    ui->catScroll2->viewport()->setMouseTracking(true);

    // 내비 버튼 연결
    connect(ui->navHome,     &QPushButton::clicked, this, &HomeWidget::on_navHome_clicked);
    connect(ui->navSearch,   &QPushButton::clicked, this, &HomeWidget::on_navSearch_clicked);
    connect(ui->navFavorite, &QPushButton::clicked, this, &HomeWidget::on_navFavorite_clicked);
    connect(ui->navOrder,    &QPushButton::clicked, this, &HomeWidget::on_navOrder_clicked);
    connect(ui->navMy,       &QPushButton::clicked, this, &HomeWidget::on_navMy_clicked);
    connect(ui->btnSearch,   &QPushButton::clicked, this, &HomeWidget::on_btnSearch_clicked);

    // 서버 메인 홈 데이터 수신
    connect(m_network, &NetworkManager::onMainHomeReceived,
            this, &HomeWidget::onMainHomeReceived);
}

HomeWidget::~HomeWidget() { delete ui; }

void HomeWidget::setUserName(const QString &userName) { m_userName = userName; }
void HomeWidget::setAddress(const QString &address)
{
    m_address = address;
    ui->label_address->setText(address.isEmpty() ? "주소 없음" : address);
}

// ============================================================
// 서버 데이터 수신 → UI 채우기
// ============================================================
void HomeWidget::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                     QList<TopStoreInfoQt> topStores)
{
    qDebug() << "[HomeWidget] 카테고리:" << categories.size()
             << "가게:" << topStores.size();

    // ── 카테고리 채우기 ──
    QLayout *catLayout = ui->catContent1->layout();
    QLayoutItem *child;
    while ((child = catLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const CategoryInfoQt &cat : categories)
        catLayout->addWidget(makeCategoryItem(cat.id, cat.name, cat.iconPath));
    static_cast<QHBoxLayout*>(catLayout)->addStretch();

    // ── 가게 카드 채우기 ──
    QLayout *storeLayout = ui->storeContent->layout();
    while ((child = storeLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const TopStoreInfoQt &store : topStores)
        storeLayout->addWidget(makeStoreCard(store));
    static_cast<QVBoxLayout*>(storeLayout)->addStretch();
}

// ============================================================
// 카테고리 아이템 위젯
// ============================================================
QWidget* HomeWidget::makeCategoryItem(int id, const QString &name, const QString &iconPath)
{
    Q_UNUSED(iconPath)

    QWidget *item = new QWidget();
    item->setFixedWidth(64);
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
        "QPushButton:pressed { background:#c5d8ff; }"
    );
    // TODO: iconPath로 실제 이미지 로드
    imgBtn->setText(categoryEmoji(name));

    QLabel *nameLabel = new QLabel(name.isEmpty() ? "-" : name);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size:11px; color:#333333; background:transparent;");
    nameLabel->setWordWrap(false);

    vl->addWidget(imgBtn, 0, Qt::AlignHCenter);
    vl->addWidget(nameLabel, 0, Qt::AlignHCenter);

    connect(imgBtn, &QPushButton::clicked, this, [this, id, name]() {
        emit categorySelected(id, name);
    });

    return item;
}

// ============================================================
// 가게 카드 위젯
// ============================================================
QWidget* HomeWidget::makeStoreCard(const TopStoreInfoQt &store)
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
    imgLabel->setText(categoryEmoji(store.category));
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

    QLabel *nameLabel = new QLabel(store.storeName.isEmpty() ? "-" : store.storeName);
    nameLabel->setStyleSheet("font-size:17px; font-weight:bold; color:#111111;");

    QLabel *metaLabel = new QLabel(
        QString("⭐ %1 (%2)  ·  %3  ·  %4")
            .arg(store.rating, 0, 'f', 1)
            .arg(store.reviewCount)
            .arg(formatDeliveryFee(store.deliveryFee))
            .arg(store.deliveryTimeRange)
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
QString HomeWidget::placeholderColorForCard(const QString &cat)
{
    if (cat == "중식") return "#fde8d8"; if (cat == "일식") return "#d8eafd";
    if (cat == "치킨") return "#fdf5d8"; if (cat == "한식") return "#d8fde4";
    if (cat == "양식") return "#fdd8d8"; if (cat == "카페") return "#ede8fd";
    if (cat == "베이커리") return "#fdf0d8"; if (cat == "분식") return "#fde8f0";
    return "#f0f0f0";
}
QString HomeWidget::categoryEmoji(const QString &cat)
{
    if (cat == "한식") return "🍚"; if (cat == "중식") return "🥟";
    if (cat == "일식") return "🍱"; if (cat == "치킨") return "🍗";
    if (cat == "양식") return "🍕"; if (cat == "분식") return "🍢";
    if (cat == "돈까스") return "🥩"; return "🍽";
}

void HomeWidget::on_btnSearch_clicked()    { emit searchRequested(); }
void HomeWidget::on_navHome_clicked()      {}
void HomeWidget::on_navSearch_clicked()    { emit searchRequested(); }
void HomeWidget::on_navFavorite_clicked()  { emit favoriteRequested(); }
void HomeWidget::on_navOrder_clicked()     { emit orderListRequested(); }
void HomeWidget::on_navMy_clicked()        { emit mypageRequested(); }
