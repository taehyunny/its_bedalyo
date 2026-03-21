#include "SearchWidget.h"
#include "ui_searchwidget.h"
#include "UserSession.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
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

    // ── 버튼 연결 ──
    connect(ui->btnBack,      &QPushButton::clicked, this, &SearchWidget::on_btnBack_clicked);
    connect(ui->btnSearch,    &QPushButton::clicked, this, &SearchWidget::on_btnSearch_clicked);
    connect(ui->btnDeleteAll, &QPushButton::clicked, this, &SearchWidget::on_btnDeleteAll_clicked);

    // ── 내비바 연결 ──
    connect(ui->navHome,     &QPushButton::clicked, this, &SearchWidget::on_navHome_clicked);
    connect(ui->navSearch,   &QPushButton::clicked, this, &SearchWidget::on_navSearch_clicked);
    connect(ui->navFavorite, &QPushButton::clicked, this, &SearchWidget::on_navFavorite_clicked);
    connect(ui->navOrder,    &QPushButton::clicked, this, &SearchWidget::on_navOrder_clicked);
    connect(ui->navMy,       &QPushButton::clicked, this, &SearchWidget::on_navMy_clicked);

    // ── 서버 응답 연결 ──
    // TODO: NetworkManager에 onSearchWidgetReceived 시그널 추가 후 연결
    // connect(m_network, &NetworkManager::onSearchWidgetReceived,
    //         this, &SearchWidget::onSearchWidgetReceived);

    // ── 초기 상태: 최근 검색어 섹션 숨김 (데이터 수신 후 show()) ──
    ui->recentSection->hide();
    ui->divider->hide();

    // ── 임시 더미 데이터 (서버 연동 전 UI 확인용) ──
    // TODO: 실제 서버 연동 후 아래 블록 제거
    QList<QString> dummyPopular = {
        "떡볶이", "치킨", "버거", "마라탕", "김밥"
    };
    QList<RecentSearchItem> dummyRecent = {
        { 2, "떡볶이", "03.21" },
        { 1, "마라탕", "03.21" }
    };
    buildPopularList(dummyPopular);
    buildRecentList(dummyRecent);
}

SearchWidget::~SearchWidget() { delete ui; }

// ============================================================
// 화면 진입 시 서버에 검색 위젯 데이터 요청
// MainWindow::onSearchRequested() 에서 호출
// REQ_RESEACH_WIDGET(2108) + userId 전송
// ============================================================
void SearchWidget::loadSearchData()
{
    // TODO: NetworkManager에 sendSearchWidget(userId) 추가 후 활성화
    // SearchWidgetReqDTO dto;
    // dto.userId = UserSession::instance().userId.toStdString();
    // m_network->sendSearchWidget(dto);

    qDebug() << "[SearchWidget] loadSearchData() 호출 - userId:"
             << UserSession::instance().userId;
}

// ============================================================
// 인기 검색어 목록 빌드 (5개 고정, 2열 그리드)
// 좌: 1~5위 / 우: 6~10위 → 현재는 1~5위만 사용
// ============================================================
void SearchWidget::buildPopularList(const QList<QString> &keywords)
{
    clearLayout(ui->popularGridLayout);

    // 2열 그리드: 홀수 인덱스는 왼쪽, 짝수 인덱스는 오른쪽
    // 현재 1~5개만 받으므로 왼쪽 열만 사용
    for (int i = 0; i < keywords.size() && i < 5; ++i) {
        QWidget *row = new QWidget();
        row->setStyleSheet("background: transparent;");
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);

        // 순위 번호
        QLabel *rankLabel = new QLabel(QString::number(i + 1));
        rankLabel->setFixedWidth(20);
        rankLabel->setAlignment(Qt::AlignCenter);
        // 1~3위는 파란색, 나머지는 검정
        QString rankColor = (i < 3) ? "#1565c0" : "#333333";
        rankLabel->setStyleSheet(
            QString("font-size:14px; font-weight:bold; color:%1; background:transparent;")
                .arg(rankColor)
        );

        // 검색어 버튼 (클릭 시 자동 입력 + 검색 실행)
        QPushButton *kwBtn = new QPushButton(keywords[i]);
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

        // 클릭 시 검색창 자동 입력 후 검색 실행
        connect(kwBtn, &QPushButton::clicked, this, [this, keyword = keywords[i]]() {
            executeSearch(keyword);
        });

        hl->addWidget(rankLabel);
        hl->addWidget(kwBtn);
        hl->addStretch();

        // 행 높이 고정 (간격 균등)
        row->setFixedHeight(44);

        // 그리드 배치: 한 열만 사용 (0번 열)
        ui->popularGridLayout->addWidget(row, i, 0);
    }
}

// ============================================================
// 최근 검색어 목록 빌드
// 새로운 검색어가 위로 추가되는 구조 → 리스트 그대로 삽입
// (서버에서 최신순으로 내려준다고 가정)
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

    for (const RecentSearchItem &item : items) {
        ui->recentListLayout->addWidget(makeRecentItemWidget(item));
    }
}

// ============================================================
// 최근 검색어 단건 위젯 생성
// [ 🕐 검색어        날짜  X ]
// ============================================================
QWidget* SearchWidget::makeRecentItemWidget(const RecentSearchItem &item)
{
    QWidget *row = new QWidget();
    row->setObjectName(QString("recentRow_%1").arg(item.searchId));
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

    // 검색어 버튼 (클릭 시 자동 입력 + 검색 실행)
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

    // 날짜
    QLabel *dateLabel = new QLabel(item.date);
    dateLabel->setStyleSheet("font-size:12px; color:#aaaaaa; background:transparent;");
    dateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

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
    connect(delBtn, &QPushButton::clicked, this, [this, id = item.searchId, kw = item.keyword]() {
        deleteRecentItem(id, kw);
    });

    hl->addWidget(iconLabel);
    hl->addWidget(kwBtn, 1);
    hl->addWidget(dateLabel);
    hl->addWidget(delBtn);

    // 하단 구분선
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet("color:#f0f0f0; background:#f0f0f0;");

    QWidget *wrapper = new QWidget();
    wrapper->setObjectName(QString("recentWrapper_%1").arg(item.searchId));
    wrapper->setStyleSheet("background:transparent;");
    QVBoxLayout *vl = new QVBoxLayout(wrapper);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    vl->addWidget(row);
    vl->addWidget(line);

    return wrapper;
}

// ============================================================
// 검색 실행 (검색창 자동 입력 후 시그널 emit)
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

    // TODO: REQ_RESEARCH_ADD(2112) 전송 (서버 최근 검색어 저장)
    // SearchAddReqDTO dto;
    // dto.userId  = UserSession::instance().userId.toStdString();
    // dto.keyword = keyword.toStdString();
    // m_network->sendSearchAdd(dto);

    emit searchRequested(keyword);
}

// ============================================================
// 최근 검색어 단건 삭제
// UI 즉시 제거 → REQ_RESEARCH_DELETE(2110) 전송
// ============================================================
void SearchWidget::deleteRecentItem(int searchId, const QString &keyword)
{
    // UI에서 해당 행 제거
    QWidget *wrapper = ui->recentContainer->findChild<QWidget*>(
        QString("recentWrapper_%1").arg(searchId));
    if (wrapper) {
        ui->recentListLayout->removeWidget(wrapper);
        wrapper->deleteLater();
    }

    // m_recentItems 에서도 제거
    m_recentItems.removeIf([searchId](const RecentSearchItem &i) {
        return i.searchId == searchId;
    });

    // 모두 삭제됐으면 섹션 숨김
    if (m_recentItems.isEmpty()) {
        ui->recentSection->hide();
        ui->divider->hide();
    }

    qDebug() << "[SearchWidget] 단건 삭제 - id:" << searchId << "keyword:" << keyword;

    // TODO: REQ_RESEARCH_DELETE(2110) 전송
    // SearchDeleteReqDTO dto;
    // dto.searchId = searchId;
    // dto.userId   = UserSession::instance().userId.toStdString();
    // m_network->sendSearchDelete(dto);
}

// ============================================================
// 전체 삭제
// ============================================================
void SearchWidget::on_btnDeleteAll_clicked()
{
    clearLayout(ui->recentListLayout);
    m_recentItems.clear();
    ui->recentSection->hide();
    ui->divider->hide();

    qDebug() << "[SearchWidget] 전체 삭제 - userId:" << UserSession::instance().userId;

    // TODO: REQ_RESEARCH_DEL_ALL(2114) 전송
    // SearchDelAllReqDTO dto;
    // dto.userId = UserSession::instance().userId.toStdString();
    // m_network->sendSearchDeleteAll(dto);
}

// ============================================================
// 버튼 슬롯
// ============================================================
void SearchWidget::on_btnBack_clicked()    { emit backRequested(); }
void SearchWidget::on_btnSearch_clicked()  { executeSearch(ui->searchEdit->text().trimmed()); }
void SearchWidget::onSearchEditReturnPressed() { executeSearch(ui->searchEdit->text().trimmed()); }

void SearchWidget::on_navHome_clicked()      { emit backRequested(); }
void SearchWidget::on_navSearch_clicked()    {}
void SearchWidget::on_navFavorite_clicked()  { emit favoriteRequested(); }
void SearchWidget::on_navOrder_clicked()     { emit orderListRequested(); }
void SearchWidget::on_navMy_clicked()        { emit mypageRequested(); }

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
