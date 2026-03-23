#include "searchwidget.h"
#include "ui_searchwidget.h"
#include "UserSession.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QDebug>

// ============================================================
// 생성자
// ============================================================
SearchWidget::SearchWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── Enter 키로도 검색 실행 ──
    connect(ui->searchEdit, &QLineEdit::returnPressed,
            this, &SearchWidget::onSearchEditReturnPressed);

    // ── 서버 응답 연결 ──
    connect(m_network, &NetworkManager::onSearchWidgetReceived,
            this, &SearchWidget::onSearchWidgetReceived);

    // ── 초기 상태: 최근 검색어 섹션 숨김 ──
    ui->recentSection->hide();
    ui->divider->hide();
}

SearchWidget::~SearchWidget() { delete ui; }

// ============================================================
// 화면 진입 시 서버에 검색 위젯 데이터 요청
// REQ_RESEACH_WIDGET(2108) + userId 전송
// 인기 검색어 옆 시간은 서버에서 받지 않고 진입 시점의 로컬 시간 표시
// ============================================================
void SearchWidget::loadSearchData()
{
    qDebug() << "[SearchWidget] loadSearchData() - userId:"
             << UserSession::instance().userId;

    // ── 인기 검색어 옆 업데이트 시간 표시 (오전/오후 HH:MM 형식) ──
    // 예: "오후 11:07 업데이트"
    QDateTime now = QDateTime::currentDateTime();
    QString ampm = (now.time().hour() < 12) ? "오전" : "오후";
    int hour12 = now.time().hour() % 12;
    if (hour12 == 0) hour12 = 12;
    QString timeStr = QString("%1 %2:%3 업데이트")
                        .arg(ampm)
                        .arg(hour12)
                        .arg(now.time().minute(), 2, 10, QChar('0'));
    ui->labelPopularTime->setText(timeStr);

    m_network->sendSearchWidget(UserSession::instance().userId);
}

// ============================================================
// 서버 응답 수신 → 인기검색어 + 최근검색어 UI 빌드
// ============================================================
void SearchWidget::onSearchWidgetReceived(QList<PopularKeywordQt> popular,
                                          QList<RecentSearchQt> recent)
{
    qDebug() << "[SearchWidget] 서버 응답 - 인기:" << popular.size()
             << "최근:" << recent.size();

    buildPopularList(popular);

    QList<RecentSearchItem> recentItems;
    for (const RecentSearchQt &r : recent) {
        RecentSearchItem item;
        item.historyId = r.historyId;
        item.keyword   = r.keyword;
        recentItems.append(item);
    }
    buildRecentList(recentItems);
}

// ============================================================
// 인기 검색어 목록 빌드
// TODO: 현재 상위 5개만 표시 중 — 서버에서 10개 오더라도 5개만 렌더링
//       추후 UI 확장 시 && i < 5 조건 제거 또는 개수 조정 필요
// ============================================================
void SearchWidget::buildPopularList(const QList<PopularKeywordQt> &keywords)
{
    clearLayout(ui->popularGridLayout);

    // TODO: 현재 5개 고정 표시 — 전체 표시로 변경 시 "&& i < 5" 제거
    for (int i = 0; i < keywords.size() && i < 5; ++i) {
        QWidget *row = new QWidget();
        row->setStyleSheet("background: transparent;");
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);

        // 순위 번호 (1~3위: 파란색, 4~5위: 검정)
        QLabel *rankLabel = new QLabel(QString::number(keywords[i].rank));
        rankLabel->setFixedWidth(20);
        rankLabel->setAlignment(Qt::AlignCenter);
        QString rankColor = (i < 3) ? "#1565c0" : "#333333";
        rankLabel->setStyleSheet(
            QString("font-size:14px; font-weight:bold; color:%1; background:transparent;")
                .arg(rankColor)
        );

        // 검색어 버튼
        QPushButton *kwBtn = new QPushButton(keywords[i].keyword);
        kwBtn->setStyleSheet(
            "QPushButton {"
            "  background:transparent; border:none;"
            "  font-size:14px; color:#222222;"
            "  text-align:left; padding:0;"
            "}"
            "QPushButton:hover { color:#1565c0; }"
            "QPushButton:pressed { color:#0d47a1; }"
        );
        kwBtn->setCursor(Qt::PointingHandCursor);

        connect(kwBtn, &QPushButton::clicked, this, [this, keyword = keywords[i].keyword]() {
            executeSearch(keyword);
        });

        hl->addWidget(rankLabel);
        hl->addWidget(kwBtn);
        hl->addStretch();

        row->setFixedHeight(44);
        ui->popularGridLayout->addWidget(row, i, 0);
    }
}

// ============================================================
// 최근 검색어 목록 빌드
// ============================================================
void SearchWidget::buildRecentList(const QList<RecentSearchItem> &items)
{
    m_recentItems = items;
    clearLayout(ui->recentListLayout);

    if (items.isEmpty()) {
        ui->recentSection->hide();
        ui->divider->hide();
        return;
    }

    ui->divider->show();
    ui->recentSection->show();

    for (const RecentSearchItem &item : items)
        ui->recentListLayout->addWidget(makeRecentItemWidget(item));
}

// ============================================================
// 최근 검색어 단건 위젯 생성
// [ 🕐 검색어   X ]
// TODO: 서버에서 searchDate 내려주면 날짜 표시 추가 예정
//       RecentSearchItem에 QString searchDate 필드 추가 후 dateLabel 활성화
// ============================================================
QWidget* SearchWidget::makeRecentItemWidget(const RecentSearchItem &item)
{
    QWidget *row = new QWidget();
    row->setObjectName(QString("recentRow_%1").arg(item.historyId));
    row->setFixedHeight(48);
    row->setStyleSheet("background: transparent;");

    QHBoxLayout *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(10);

    // 시계 아이콘
    QLabel *iconLabel = new QLabel("🕐");
    iconLabel->setFixedWidth(20);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size:14px; background:transparent; color:#aaaaaa;");

    // 검색어 버튼
    QPushButton *kwBtn = new QPushButton(item.keyword);
    kwBtn->setStyleSheet(
        "QPushButton {"
        "  background:transparent; border:none;"
        "  font-size:14px; color:#333333;"
        "  text-align:left; padding:0;"
        "}"
        "QPushButton:hover { color:#1565c0; }"
        "QPushButton:pressed { color:#0d47a1; }"
    );
    kwBtn->setCursor(Qt::PointingHandCursor);
    connect(kwBtn, &QPushButton::clicked, this, [this, keyword = item.keyword]() {
        executeSearch(keyword);
    });

    // TODO: 서버에서 searchDate 내려주면 아래 dateLabel 활성화
    // QLabel *dateLabel = new QLabel(item.searchDate);
    // dateLabel->setStyleSheet("font-size:12px; color:#aaaaaa; background:transparent;");
    // dateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // X 삭제 버튼
    QPushButton *delBtn = new QPushButton("✕");
    delBtn->setFixedSize(24, 24);
    delBtn->setStyleSheet(
        "QPushButton {"
        "  background:transparent; border:none;"
        "  font-size:12px; color:#bbbbbb; padding:0;"
        "}"
        "QPushButton:hover { color:#e53935; }"
        "QPushButton:pressed { color:#b71c1c; }"
    );
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, [this, id = item.historyId, kw = item.keyword]() {
        deleteRecentItem(id, kw);
    });

    hl->addWidget(iconLabel);
    hl->addWidget(kwBtn, 1);
    // TODO: hl->addWidget(dateLabel); // searchDate 확정 후 활성화
    hl->addWidget(delBtn);

    // 하단 구분선
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet("color:#f0f0f0; background:#f0f0f0;");

    QWidget *wrapper = new QWidget();
    wrapper->setObjectName(QString("recentWrapper_%1").arg(item.historyId));
    wrapper->setStyleSheet("background:transparent;");
    QVBoxLayout *vl = new QVBoxLayout(wrapper);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    vl->addWidget(row);
    vl->addWidget(line);

    return wrapper;
}

// ============================================================
// 검색 실행
// ============================================================
void SearchWidget::executeSearch(const QString &keyword)
{
    if (keyword.trimmed().isEmpty()) {
        QMessageBox::warning(this, "검색어 없음", "검색어를 입력해주세요.");
        ui->searchEdit->setFocus();
        return;
    }

    ui->searchEdit->setText(keyword);

    qDebug() << "[SearchWidget] 검색 실행:" << keyword;

    // REQ_RESEARCH_ADD(2112) 전송 — 서버 최근 검색어 저장
    m_network->sendSearchAdd(UserSession::instance().userId, keyword);

    emit searchRequested(keyword);
}

// ============================================================
// 최근 검색어 단건 삭제
// UI 즉시 제거 (낙관적 업데이트) → REQ_RESEARCH_DELETE(2110) 전송
// ============================================================
void SearchWidget::deleteRecentItem(int historyId, const QString &keyword)
{
    Q_UNUSED(keyword)

    QWidget *wrapper = ui->recentContainer->findChild<QWidget*>(
        QString("recentWrapper_%1").arg(historyId));
    if (wrapper) {
        ui->recentListLayout->removeWidget(wrapper);
        wrapper->deleteLater();
    }

    m_recentItems.removeIf([historyId](const RecentSearchItem &i) {
        return i.historyId == historyId;
    });

    if (m_recentItems.isEmpty()) {
        ui->recentSection->hide();
        ui->divider->hide();
    }

    qDebug() << "[SearchWidget] 단건 삭제 - historyId:" << historyId;

    m_network->sendSearchDelete(UserSession::instance().userId, historyId);
}

// ============================================================
// 전체 삭제
// UI 즉시 제거 (낙관적 업데이트) → REQ_RESEARCH_DEL_ALL(2114) 전송
// ============================================================
void SearchWidget::on_btnDeleteAll_clicked()
{
    clearLayout(ui->recentListLayout);
    m_recentItems.clear();
    ui->recentSection->hide();
    ui->divider->hide();

    qDebug() << "[SearchWidget] 전체 삭제 - userId:" << UserSession::instance().userId;

    m_network->sendSearchDeleteAll(UserSession::instance().userId);
}

// ============================================================
// 버튼 슬롯
// ============================================================
void SearchWidget::on_btnBack_clicked()        { emit backRequested(); }
void SearchWidget::on_btnSearch_clicked()      { executeSearch(ui->searchEdit->text().trimmed()); }
void SearchWidget::onSearchEditReturnPressed() { executeSearch(ui->searchEdit->text().trimmed()); }

void SearchWidget::on_navHome_clicked()        { emit backRequested(); }
void SearchWidget::on_navSearch_clicked()      {}
void SearchWidget::on_navFavorite_clicked()    { emit favoriteRequested(); }
void SearchWidget::on_navOrder_clicked()       { emit orderListRequested(); }
void SearchWidget::on_navMy_clicked()          { emit mypageRequested(); }

// ============================================================
// 레이아웃 전체 비우기
// ============================================================
void SearchWidget::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}
