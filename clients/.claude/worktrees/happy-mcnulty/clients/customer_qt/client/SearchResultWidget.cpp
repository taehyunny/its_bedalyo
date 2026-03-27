#include "searchresultwidget.h"
#include "storeutils.h"
#include "ui_searchresultwidget.h"
#include "UserSession.h"
#include <QFrame>
#include <QLocale>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>

// ============================================================
// 생성자
// ============================================================
SearchResultWidget::SearchResultWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchResultWidget)
    , m_network(network)
{
    ui->setupUi(this);

    connect(ui->btnBack, &QPushButton::clicked,
            this, &SearchResultWidget::on_btnBack_clicked);

    // 검색 버튼은 UI 장식용 — 기능 없음
    ui->btnSearch->setEnabled(false);

    // ── 서버 응답 연결 ──
    connect(m_network, &NetworkManager::onSearchResultReceived,
            this, &SearchResultWidget::onSearchResultReceived);
}

SearchResultWidget::~SearchResultWidget() { delete ui; }

// ============================================================
// 검색 실행 — MainWindow에서 호출
// ============================================================
void SearchResultWidget::search(const QString &keyword)
{
    m_keyword = keyword.trimmed();
    ui->searchEdit->setText(m_keyword);
    ui->labelResultCount->setText("검색 결과를 불러오는 중...");

    clearStoreList();

    qDebug() << "[SearchResultWidget] 검색 요청:" << m_keyword;

    // ── REQ_SEARCH_STORE(2116) 전송 ──
    m_network->sendSearchStore(m_keyword);
}

// ============================================================
// 서버 응답 수신 → UI 업데이트
// TODO: 서버 연동 후 더미 데이터 블록 제거 및 주석 해제
// ============================================================
void SearchResultWidget::onSearchResultReceived(QList<TopStoreInfoQt> stores)
{
    buildStoreList(stores);
}

// ============================================================
// 매장 리스트 빌드
// ============================================================
void SearchResultWidget::buildStoreList(const QList<TopStoreInfoQt> &stores)
{
    clearStoreList();

    if (stores.isEmpty()) {
        showEmpty();
        return;
    }

    ui->labelResultCount->setText(
        QString("'%1' 검색 결과 %2개").arg(m_keyword).arg(stores.size())
    );

    for (const TopStoreInfoQt &store : stores)
        ui->storeListLayout->addWidget(makeStoreCard(store));

    static_cast<QVBoxLayout*>(ui->storeListLayout)->addStretch();
}

// ============================================================
// 검색 결과 없음
// ============================================================
void SearchResultWidget::showEmpty()
{
    ui->labelResultCount->setText(QString("'%1' 검색 결과 없음").arg(m_keyword));

    QLabel *emptyLabel = new QLabel(
        QString("'%1'에 대한\n검색 결과가 없습니다.").arg(m_keyword)
    );
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet("font-size:15px; color:#aaaaaa; padding:60px;");
    ui->storeListLayout->addWidget(emptyLabel);
    static_cast<QVBoxLayout*>(ui->storeListLayout)->addStretch();
}

// ============================================================
// 매장 카드 위젯
// 클릭은 카드 내부 투명 QPushButton 오버레이로 처리
// eventFilter 없이 람다로 storeSelected emit
// ============================================================
QWidget* SearchResultWidget::makeStoreCard(const TopStoreInfoQt &store)
{
    QWidget *card = new QWidget();
    card->setStyleSheet("background:#ffffff; border-radius:8px;");

    QVBoxLayout *vl = new QVBoxLayout(card);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    // 썸네일 이미지 (이모지 플레이스홀더)
    QLabel *imgLabel = new QLabel();
    imgLabel->setFixedHeight(180);
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setStyleSheet(
        QString("background-color:%1; font-size:48px; border-radius:8px 8px 0 0;")
            .arg(StoreUtils::placeholderColor(store.category))
    );
    imgLabel->setText(StoreUtils::categoryEmoji(store.category));
    vl->addWidget(imgLabel);

    // 정보 영역
    QWidget *info = new QWidget();
    info->setStyleSheet("background:#ffffff;");
    QVBoxLayout *il = new QVBoxLayout(info);
    il->setContentsMargins(14, 10, 14, 14);
    il->setSpacing(4);

    // 카테고리 뱃지
    QLabel *catLabel = new QLabel(store.category);
    catLabel->setStyleSheet(
        "font-size:11px; color:#1565c0; background:#e8f0ff;"
        "border-radius:4px; padding:2px 8px; font-weight:bold;"
    );
    catLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    // 매장명
    QLabel *nameLabel = new QLabel(store.storeName.isEmpty() ? "-" : store.storeName);
    nameLabel->setStyleSheet("font-size:17px; font-weight:bold; color:#111111;");

    // 별점·배달비·시간
    QLabel *metaLabel = new QLabel(
        QString("⭐ %1 (%2)  ·  %3  ·  %4")
            .arg(store.rating, 0, 'f', 1)
            .arg(store.reviewCount)
            .arg(StoreUtils::formatDeliveryFee(store.deliveryFee))
            .arg(store.deliveryTimeRange)
    );
    metaLabel->setStyleSheet("font-size:13px; color:#555555;");

    // 최소주문
    QLabel *minLabel = new QLabel("최소주문 " + StoreUtils::formatWon(store.minOrderAmount));
    minLabel->setStyleSheet("font-size:12px; color:#888888;");

    il->addWidget(catLabel);
    il->addWidget(nameLabel);
    il->addWidget(metaLabel);
    il->addWidget(minLabel);
    vl->addWidget(info);

    // ── 카드 전체를 덮는 클릭 버튼 (투명 오버레이) ──
    // eventFilter 없이 람다로 클릭 처리
    QPushButton *clickOverlay = new QPushButton(card);
    clickOverlay->setStyleSheet(
        "QPushButton { background:transparent; border:none; }"
        "QPushButton:hover { background:rgba(0,0,0,0.03); }"
        "QPushButton:pressed { background:rgba(0,0,0,0.07); }"
    );
    clickOverlay->setCursor(Qt::PointingHandCursor);
    clickOverlay->raise();

    connect(clickOverlay, &QPushButton::clicked, this, [this, storeId = store.storeId]() {
        // TODO: REQ_RESEARCH_ADD(2112) 카테고리 점수 올리기 (서버팀 DTO 확정 후)
        emit storeSelected(storeId);
    });

    return card;
}

// ============================================================
// 리스트 초기화
// ============================================================
void SearchResultWidget::clearStoreList()
{
    QLayout *layout = ui->storeListLayout;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}

// ============================================================
// 버튼 슬롯
// ============================================================
void SearchResultWidget::on_btnBack_clicked() { emit backRequested(); }

// ============================================================
// 헬퍼
// ============================================================







