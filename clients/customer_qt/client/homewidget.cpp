#include "homewidget.h"
#include "storeutils.h"
#include "cartsession.h"
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
// CatScrollFilter
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
            int deltaX = e->pos().x() - m_lastPos.x();
            m_target->horizontalScrollBar()->setValue(
                m_target->horizontalScrollBar()->value() - deltaX);
            int deltaY = e->pos().y() - m_lastPos.y();
            m_target->verticalScrollBar()->setValue(
                m_target->verticalScrollBar()->value() - deltaY);
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

    // CartBar는 MainWindow가 직접 관리하므로 여기선 아무것도 안 함

    auto *filter1 = new CatScrollFilter(ui->catScroll1);
    ui->catScroll1->viewport()->installEventFilter(filter1);
    ui->catScroll1->viewport()->setMouseTracking(true);

    auto *filter2 = new CatScrollFilter(ui->catScroll2);
    ui->catScroll2->viewport()->installEventFilter(filter2);
    ui->catScroll2->viewport()->setMouseTracking(true);

    auto *storeFilter = new CatScrollFilter(ui->storeScrollArea);
    ui->storeScrollArea->viewport()->installEventFilter(storeFilter);
    ui->storeScrollArea->viewport()->setMouseTracking(true);

    connect(m_network, &NetworkManager::onMainHomeReceived,
            this, &HomeWidget::onMainHomeReceived);

    connect(m_network, &NetworkManager::onHeartbeatReceived,
            this, &HomeWidget::onMainHomeReceived);

    // CartBar: 클릭 시 cartRequested 시그널 발사
    connect(ui->cartBar, &CartBarWidget::cartRequested,
            this, &HomeWidget::cartRequested);
}

HomeWidget::~HomeWidget() { delete ui; }

void HomeWidget::setUserName(const QString &userName) { m_userName = userName; }

void HomeWidget::setAddress(const QString &address)
{
    m_address = address;
    ui->btnAddress->setText(address.isEmpty() ? "주소를 설정해주세요 ▾" : address + " ▾");
}

void HomeWidget::on_btnAddress_clicked()
{
    emit addressRequested();
}

// ============================================================
// 서버 데이터 수신
// ============================================================
void HomeWidget::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                     QList<TopStoreInfoQt> topStores)
{
    qDebug() << "[HomeWidget] 카테고리:" << categories.size()
             << "가게:" << topStores.size();

    QLayout *catLayout = ui->catContent1->layout();
    QLayoutItem *child;
    while ((child = catLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const CategoryInfoQt &cat : categories)
        catLayout->addWidget(makeCategoryItem(cat.id, cat.name, cat.iconPath));
    static_cast<QHBoxLayout*>(catLayout)->addStretch();

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
// 카테고리 아이템
// ============================================================
QWidget* HomeWidget::makeCategoryItem(int id, const QString &name, const QString &iconPath)
{
    Q_UNUSED(iconPath)

    QWidget *item = new QWidget();
    item->setFixedSize(64, 80);
    item->setStyleSheet("background: transparent;");

    QVBoxLayout *vl = new QVBoxLayout(item);
    vl->setContentsMargins(0, 4, 0, 4);
    vl->setSpacing(4);
    vl->setAlignment(Qt::AlignHCenter);

    QPushButton *imgBtn = new QPushButton();
    imgBtn->setFixedSize(52, 52);
    imgBtn->setStyleSheet(
        "QPushButton { background:#f0f4ff; border-radius:26px; border:none; font-size:24px; }"
        "QPushButton:hover { background:#dce8ff; }"
        "QPushButton:pressed { background:#c5d8ff; }"
    );
    imgBtn->setText(StoreUtils::categoryEmoji(name));

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
// 가게 카드
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
            .arg(StoreUtils::placeholderColor(store.category))
    );
    imgLabel->setText(StoreUtils::categoryEmoji(store.category));
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
            .arg(StoreUtils::formatDeliveryFee(store.deliveryFee))
            .arg(store.deliveryTimeRange)
    );
    metaLabel->setStyleSheet("font-size:13px; color:#555555;");

    QLabel *minOrderLabel = new QLabel("최소주문 " + StoreUtils::formatWon(store.minOrderAmount));
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

    card->setProperty("storeId", store.storeId);
    card->setCursor(Qt::PointingHandCursor);
    card->installEventFilter(this);
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

void HomeWidget::on_btnSearch_clicked()   { emit searchRequested(); }
void HomeWidget::on_navHome_clicked()     { m_network->sendHeartbeat(); }
void HomeWidget::on_navSearch_clicked()   { emit searchRequested(); }
void HomeWidget::on_navFavorite_clicked() { emit favoriteRequested(); }
void HomeWidget::on_navOrder_clicked()    { emit orderListRequested(); }
void HomeWidget::on_navMy_clicked()       { emit mypageRequested(); }

// ============================================================
// CartBar 갱신 — CartSession 상태 보고 show/hide
// ============================================================
void HomeWidget::updateCartBar()
{
    ui->cartBar->updateCartUI(); // CartBarWidget 내부에서 show/hide 처리
}
