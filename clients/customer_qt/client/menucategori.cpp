#include "menucategori.h"
#include "storeutils.h"
#include "ui_menucategori.h"
#include <QFrame>
#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDebug>
#include <functional>

// ============================================================
// CatScrollFilter - 가로 + 세로 드래그 스크롤 모두 지원
// categoryTabScroll(가로), storeScrollArea(세로) 둘 다 사용
// ============================================================
class CatScrollFilter : public QObject {
public:
    CatScrollFilter(QScrollArea *target, QObject *parent = nullptr)
        : QObject(parent), m_target(target) {}

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

            // ── 가로 드래그 ──
            int deltaX = e->pos().x() - m_lastPos.x();
            m_target->horizontalScrollBar()->setValue(
                m_target->horizontalScrollBar()->value() - deltaX);

            // ── 세로 드래그 ──
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
    bool         m_dragging = false;
    QPoint       m_lastPos;
};


// ============================================================
// ClickableLabel - 드래그 vs 클릭 구분 버전
//
// Press 시점에 콜백을 바로 호출하지 않고,
// Release 시점에 이동 거리가 5px 미만일 때만 클릭으로 인정
// → 카테고리 탭을 드래그로 스크롤할 때 탭이 선택되는 문제 해결
// ============================================================
class ClickableLabel : public QLabel {
public:
    explicit ClickableLabel(QWidget *parent = nullptr) : QLabel(parent) {}

    void setTabInfo(int id, const QString &name) {
        m_categoryId = id; m_categoryName = name;
    }
    int     categoryId()   const { return m_categoryId; }
    QString categoryName() const { return m_categoryName; }

    void setClickCallback(std::function<void(int, const QString&)> cb) {
        m_callback = cb;
    }

protected:
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton)
            m_pressPos = e->pos(); // 시작 위치만 저장, 콜백은 아직 호출 안 함
        QLabel::mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton && m_callback) {
            // 5px 이내로 움직였을 때만 클릭으로 인정
            if ((e->pos() - m_pressPos).manhattanLength() < 5)
                m_callback(m_categoryId, m_categoryName);
        }
        QLabel::mouseReleaseEvent(e);
    }

private:
    int     m_categoryId   = -1;
    QString m_categoryName;
    QPoint  m_pressPos;   // 클릭 시작 위치 (드래그 판별용)
    std::function<void(int, const QString&)> m_callback;
};

// ============================================================
// 생성자
// ============================================================
menucategori::menucategori(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menucategori)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 카테고리 탭: 가로 드래그 스크롤 ──
    auto *hFilter = new CatScrollFilter(ui->categoryTabScroll, this);
    ui->categoryTabScroll->viewport()->installEventFilter(hFilter);
    ui->categoryTabScroll->viewport()->setMouseTracking(true);

    // ── 가게 목록: 세로 드래그 스크롤 ──
    auto *storeFilter = new CatScrollFilter(ui->storeScrollArea, this);
    ui->storeScrollArea->viewport()->installEventFilter(storeFilter);
    ui->storeScrollArea->viewport()->setMouseTracking(true);

    // ── 버튼 연결 ──
    connect(ui->btnBack, &QPushButton::clicked, this, &menucategori::backRequested);
    connect(ui->btnSort, &QPushButton::clicked, this, &menucategori::onSortButtonClicked);

    // ── 가게 목록 수신 ──
    connect(m_network, &NetworkManager::onStoreListReceived,
            this, &menucategori::onStoreListReceived);
}

menucategori::~menucategori()
{
    delete ui;
}

// ============================================================
// MainWindow에서 호출 - 카테고리 진입 시
// ============================================================
void menucategori::setCategory(int categoryId, const QString &categoryName,
                                const QList<CategoryInfoQt> &categories)
{
    m_currentCategoryId   = categoryId;
    m_currentCategoryName = categoryName;

    ui->labelTitle->setText(categoryName);

    buildCategoryTabs(categories);

    m_currentSortLabel = "추천순";
    ui->btnSort->setText(m_currentSortLabel + " ▼");

    m_network->sendStoreListRequest(categoryId);
}

// ============================================================
// 카테고리 탭 빌드
// ============================================================
void menucategori::buildCategoryTabs(const QList<CategoryInfoQt> &categories)
{
    QLayout *layout = ui->categoryTabContentsLayout;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    for (const CategoryInfoQt &cat : categories) {
        ClickableLabel *tab = new ClickableLabel();
        tab->setText(cat.name);
        tab->setAlignment(Qt::AlignCenter);
        tab->setMinimumHeight(46);
        tab->setContentsMargins(14, 0, 14, 0);
        tab->setTabInfo(cat.id, cat.name);

        bool selected = (cat.id == m_currentCategoryId);
        tab->setStyleSheet(selected
            ? "QLabel { border-bottom:2.5px solid #1565c0; background:transparent;"
              "  font-size:14px; font-weight:bold; color:#1565c0; padding:0 14px; min-height:46px; }"
            : "QLabel { border-bottom:2.5px solid transparent; background:transparent;"
              "  font-size:14px; color:#888888; padding:0 14px; min-height:46px; }"
        );

        tab->setClickCallback([this](int id, const QString &name) {
            onCategoryTabClicked(id, name);
        });
        layout->addWidget(tab);
    }

    static_cast<QHBoxLayout*>(layout)->addStretch();

    for (int i = 0; i < layout->count(); ++i) {
        ClickableLabel *b = dynamic_cast<ClickableLabel*>(layout->itemAt(i)->widget());
        if (b && b->categoryId() == m_currentCategoryId) {
            ui->categoryTabScroll->ensureWidgetVisible(b);
            break;
        }
    }
}

// ============================================================
// 카테고리 탭 클릭
// ============================================================
void menucategori::onCategoryTabClicked(int categoryId, const QString &categoryName)
{
    m_currentCategoryId   = categoryId;
    m_currentCategoryName = categoryName;
    ui->labelTitle->setText(categoryName);

    QLayout *layout = ui->categoryTabContentsLayout;
    for (int i = 0; i < layout->count(); ++i) {
        ClickableLabel *b = dynamic_cast<ClickableLabel*>(layout->itemAt(i)->widget());
        if (!b) continue;
        bool sel = (b->categoryId() == categoryId);
        b->setStyleSheet(sel
            ? "QLabel { border-bottom:2.5px solid #1565c0; background:transparent;"
              "  font-size:14px; font-weight:bold; color:#1565c0; padding:0 14px; min-height:46px; }"
            : "QLabel { border-bottom:2.5px solid transparent; background:transparent;"
              "  font-size:14px; color:#888888; padding:0 14px; min-height:46px; }"
        );
    }

    m_network->sendStoreListRequest(m_currentCategoryId);
}

// ============================================================
// 추천순 ▼ → 바텀시트
// ============================================================
void menucategori::onSortButtonClicked()
{
    QDialog sheet(this);
    sheet.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    sheet.setAttribute(Qt::WA_TranslucentBackground);
    sheet.setFixedSize(this->width(), this->height());
    sheet.move(mapToGlobal(QPoint(0, 0)));

    QWidget overlay(&sheet);
    overlay.setGeometry(0, 0, sheet.width(), sheet.height());
    overlay.setStyleSheet("background: rgba(0,0,0,0.4);");
    overlay.lower();

    QWidget panel(&sheet);
    panel.setStyleSheet("background: #FFFFFF; border-radius: 16px 16px 0 0;");
    const int panelH = 260;
    panel.setGeometry(0, sheet.height() - panelH, sheet.width(), panelH);
    panel.raise();

    QVBoxLayout vl(&panel);
    vl.setContentsMargins(0, 0, 0, 0);
    vl.setSpacing(0);

    QWidget titleBar(&panel);
    titleBar.setFixedHeight(52);
    QHBoxLayout tl(&titleBar);
    tl.setContentsMargins(20, 0, 16, 0);

    QLabel titleLabel("매장 정렬", &titleBar);
    titleLabel.setStyleSheet("font-size:16px; font-weight:bold; color:#111111;");

    QPushButton closeBtn("✕", &titleBar);
    closeBtn.setFixedSize(36, 36);
    closeBtn.setStyleSheet(
        "QPushButton { background:transparent; border:none; font-size:16px; color:#555555; }"
        "QPushButton:hover { color:#111111; }"
    );
    QObject::connect(&closeBtn, &QPushButton::clicked, &sheet, &QDialog::reject);

    tl.addWidget(&titleLabel);
    tl.addStretch();
    tl.addWidget(&closeBtn);
    vl.addWidget(&titleBar);

    QFrame divLine(&panel);
    divLine.setFrameShape(QFrame::HLine);
    divLine.setStyleSheet("color:#EEEEEE;");
    divLine.setFixedHeight(1);
    vl.addWidget(&divLine);

    const QStringList sortOptions = {"추천순", "주문많은순", "별점높은순"};
    for (const QString &opt : sortOptions) {
        QPushButton *optBtn = new QPushButton(&panel);
        optBtn->setFixedHeight(56);
        bool isCurrent = (opt == m_currentSortLabel);
        optBtn->setText(isCurrent ? "✓  " + opt : "    " + opt);
        optBtn->setStyleSheet(
            QString(
                "QPushButton {"
                "  border:none; background:transparent;"
                "  font-size:15px; color:%1; %2"
                "  text-align:left; padding-left:8px;"
                "}"
                "QPushButton:hover { background:#F5F5F5; }"
            ).arg(isCurrent ? "#1565c0" : "#111111")
             .arg(isCurrent ? "font-weight:bold;" : "")
        );
        QObject::connect(optBtn, &QPushButton::clicked, [this, &sheet, opt]() {
            m_currentSortLabel = opt;
            ui->btnSort->setText(opt + " ▼");
            sheet.accept();
            // TODO: 정렬 포함 서버 재요청
        });
        vl.addWidget(optBtn);
    }

    struct OverlayFilter : public QObject {
        QDialog *m_sheet;
        OverlayFilter(QDialog *s, QObject *p) : QObject(p), m_sheet(s) {}
        bool eventFilter(QObject *, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease) {
                m_sheet->reject();
                return true;
            }
            return false;
        }
    };
    overlay.installEventFilter(new OverlayFilter(&sheet, &sheet));

    sheet.exec();
}

// ============================================================
// 가게 목록 수신 → UI 업데이트
// ============================================================
void menucategori::onStoreListReceived(QList<TopStoreInfoQt> stores)
{
    updateStoreListUI(stores);
}

void menucategori::updateStoreListUI(const QList<TopStoreInfoQt> &stores)
{
    clearLayout(ui->storeListPlaceholder);

    for (const TopStoreInfoQt &store : stores)
        ui->storeListPlaceholder->addWidget(makeStoreCard(store));

    static_cast<QVBoxLayout*>(ui->storeListPlaceholder)->addStretch();
}

// ============================================================
// 가게 카드 위젯
// ============================================================
QWidget* menucategori::makeStoreCard(const TopStoreInfoQt &store)
{
    QWidget *card = new QWidget();
    card->setStyleSheet("background:#ffffff;");

    QVBoxLayout *vl = new QVBoxLayout(card);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    QLabel *imgLabel = new QLabel();
    imgLabel->setFixedHeight(180);
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

    il->addWidget(nameLabel);
    il->addWidget(metaLabel);
    il->addWidget(minOrderLabel);
    vl->addWidget(info);

    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color:#eeeeee;");
    divider->setFixedHeight(1);
    vl->addWidget(divider);

    // eventFilter 방식으로 클릭 감지 (스크롤 필터와 충돌 없음)
    card->setProperty("storeId", store.storeId);
    card->setCursor(Qt::PointingHandCursor);
    card->installEventFilter(this);

    return card;
}

// ============================================================
// 헬퍼
// ============================================================
bool menucategori::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (w) {
            QVariant v = w->property("storeId");
            if (v.isValid()) {
                qDebug() << "[menucategori] 가게 클릭! storeId:" << v.toInt();
                emit storeSelected(v.toInt());
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void menucategori::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}
