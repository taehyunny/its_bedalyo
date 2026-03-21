#include "menucategori.h"
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
// CatScrollFilter - homewidget.cpp의 CatScrollFilter와 완전히 동일
// categoryTabScroll viewport에 설치해서 가로 드래그 스크롤 구현
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
    bool         m_dragging = false;
    QPoint       m_lastPos;
};


// ============================================================
// ClickableLabel - mousePressEvent로 클릭을 감지하는 QLabel
// Q_OBJECT 없이 함수포인터 콜백으로 클릭 처리
// ============================================================
class ClickableLabel : public QLabel {
public:
    explicit ClickableLabel(QWidget *parent = nullptr) : QLabel(parent) {}

    void setTabInfo(int id, const QString &name) {
        m_categoryId = id; m_categoryName = name;
    }
    int     categoryId()   const { return m_categoryId; }
    QString categoryName() const { return m_categoryName; }

    // 클릭 콜백 설정 (lambda로 사용)
    void setClickCallback(std::function<void(int, const QString&)> cb) {
        m_callback = cb;
    }

protected:
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton && m_callback)
            m_callback(m_categoryId, m_categoryName);
        QLabel::mousePressEvent(e);
    }

private:
    int     m_categoryId   = -1;
    QString m_categoryName;
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

    // ── 카테고리 탭: 가로 드래그 스크롤 (homewidget CatScrollFilter 완전 동일) ──
    auto *hFilter = new CatScrollFilter(ui->categoryTabScroll, this);
    ui->categoryTabScroll->viewport()->installEventFilter(hFilter);
    ui->categoryTabScroll->viewport()->setMouseTracking(true);

    // ── 버튼 연결 ──
    connect(ui->btnBack,    &QPushButton::clicked, this, &menucategori::backRequested);
    connect(ui->btnSort,    &QPushButton::clicked, this, &menucategori::onSortButtonClicked);

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
// 카테고리 탭 가로 드래그 스크롤 생성
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

    // 선택된 탭 위치로 스크롤
    for (int i = 0; i < layout->count(); ++i) {
        ClickableLabel *b = dynamic_cast<ClickableLabel*>(layout->itemAt(i)->widget());
        if (b && b->categoryId() == m_currentCategoryId) {
            ui->categoryTabScroll->ensureWidgetVisible(b);
            break;
        }
    }
}

// ============================================================
// eventFilter - 카테고리 탭(QLabel) 클릭 감지
// CatScrollFilter가 viewport 이벤트를 소비하므로
// QLabel에 직접 installEventFilter해서 클릭 처리
// ============================================================
void menucategori::onCategoryTabClicked(int categoryId, const QString &categoryName)
{
    m_currentCategoryId   = categoryId;
    m_currentCategoryName = categoryName;
    ui->labelTitle->setText(categoryName);

    // 모든 탭 스타일 업데이트
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
// 추천순 ▼ → 바텀시트 (추천순 / 주문많은순 / 별점높은순)
// ============================================================
void menucategori::onSortButtonClicked()
{
    // ── 스택 객체로 생성 → exec() 후 자동 소멸, 크래시 없음 ──
    QDialog sheet(this);
    sheet.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    sheet.setAttribute(Qt::WA_TranslucentBackground);
    sheet.setFixedSize(this->width(), this->height());
    sheet.move(mapToGlobal(QPoint(0, 0)));

    // 반투명 오버레이 (sheet 자식)
    QWidget overlay(&sheet);
    overlay.setGeometry(0, 0, sheet.width(), sheet.height());
    overlay.setStyleSheet("background: rgba(0,0,0,0.4);");
    overlay.lower();

    // 바텀시트 패널 (sheet 자식)
    QWidget panel(&sheet);
    panel.setStyleSheet("background: #FFFFFF; border-radius: 16px 16px 0 0;");
    const int panelH = 260;
    panel.setGeometry(0, sheet.height() - panelH, sheet.width(), panelH);
    panel.raise();

    QVBoxLayout vl(&panel);
    vl.setContentsMargins(0, 0, 0, 0);
    vl.setSpacing(0);

    // 타이틀 바
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

    // 정렬 옵션 3개
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

    // 오버레이 클릭 시 닫기 (EventFilter, sheet 소유)
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

    sheet.exec(); // 스택 객체 → 자동 소멸
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
            .arg(placeholderColorForCard(store.category))
    );
    imgLabel->setText(categoryEmoji(store.category));
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
            .arg(formatDeliveryFee(store.deliveryFee))
            .arg(store.deliveryTimeRange)
    );
    metaLabel->setStyleSheet("font-size:13px; color:#555555;");

    QLabel *minOrderLabel = new QLabel("최소주문 " + formatWon(store.minOrderAmount));
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

    return card;
}

// ============================================================
// 헬퍼
// ============================================================
void menucategori::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}

QString menucategori::formatWon(int amount)
    { return QLocale(QLocale::Korean).toString(amount) + "원"; }
QString menucategori::formatDeliveryFee(int fee)
    { return (fee == 0) ? "무료배달" : "배달비 " + QLocale(QLocale::Korean).toString(fee) + "원"; }
QString menucategori::placeholderColorForCard(const QString &cat)
{
    if (cat == "한식") return "#d8fde4"; if (cat == "중식") return "#fde8d8";
    if (cat == "돈까스") return "#fff3d8"; if (cat == "양식") return "#fdd8d8";
    if (cat == "치킨") return "#fdf5d8"; if (cat == "피자") return "#fde8d8";
    if (cat == "햄버거") return "#fdf0e0"; if (cat == "족발/보쌈") return "#e8fde8";
    if (cat == "도시락") return "#fdf0d8"; if (cat == "초밥/회") return "#d8eafd";
    return "#f0f0f0";
}
QString menucategori::categoryEmoji(const QString &cat)
{
    if (cat == "한식") return "🍚"; if (cat == "중식") return "🥟";
    if (cat == "돈까스") return "🥩"; if (cat == "양식") return "🍝";
    if (cat == "치킨") return "🍗"; if (cat == "피자") return "🍕";
    if (cat == "햄버거") return "🍔"; if (cat == "족발/보쌈") return "🍖";
    if (cat == "도시락") return "🍱"; if (cat == "초밥/회") return "🍣";
    return "🍽";
}
