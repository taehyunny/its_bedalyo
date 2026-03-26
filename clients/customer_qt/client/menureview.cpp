// menureview.cpp
#include "menureview.h"
#include "ui_menureview.h"
#include "NetworkManager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

// [수정] 생성자에 NetworkManager* 추가 및 시그널 연결
menureview::menureview(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menureview)
    , m_network(network)
{
    ui->setupUi(this);

    connect(ui->btn_back, &QPushButton::clicked, this, [=]() { emit backRequested(); });

    // [추가] 네트워크에서 리뷰 목록이 도착하면 onReviewsReceived 슬롯 호출
    connect(m_network, &NetworkManager::onMenuReviewsReceived,
            this, &menureview::onReviewsReceived);
}

menureview::~menureview() { delete ui; }

// [추가] 리뷰 화면 진입 시 mainwindow에서 호출하는 함수
void menureview::loadReviews(int menuId)
{
    m_menuId = menuId;
    clearReviews();
    qDebug() << "[menureview] 리뷰 요청 - menuId:" << menuId;
    m_network->sendMenuReviewRequest(menuId);  // NetworkManager에 추가 필요
}

// [추가] 네트워크 응답 수신 슬롯
void menureview::onReviewsReceived(int menuId, QList<ReviewDTO> reviews)
{
    // 현재 화면의 menuId와 다른 응답이면 무시 (경쟁 응답 방지)
    if (m_menuId != menuId) {
        qWarning() << "[menureview] menuId 불일치 무시 - 수신:" << menuId << "현재:" << m_menuId;
        return;
    }

    qDebug() << "[menureview] 리뷰 수신 - 개수:" << reviews.size();
    clearReviews();

    if (reviews.isEmpty()) {
        QLabel *emptyLabel = new QLabel("아직 등록된 리뷰가 없습니다.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #aaa; font-size: 14px; padding: 40px;");
        ui->reviewListLayout->insertWidget(0, emptyLabel);
        return;
    }

    for (const ReviewDTO &review : reviews)
        addReviewItem(review);
}

void menureview::clearReviews()
{
    // verticalSpacer(마지막 아이템)는 남겨두고 위젯만 제거
    while (ui->reviewListLayout->count() > 1) {
        QLayoutItem *child = ui->reviewListLayout->takeAt(0);
        if (child->widget()) delete child->widget();
        delete child;
    }
}

void menureview::addReviewItem(const ReviewDTO &review)
{
    QWidget *item = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(item);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(4);

    // 별점 + 사용자 ID
    QString stars = QString("⭐").repeated(qBound(0, review.rating, 5));
    QLabel *userLabel = new QLabel(QString("%1  %2").arg(stars, QString::fromStdString(review.userId)));
    userLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px;");

    // 메뉴명 태그 (있을 때만)
    if (!review.menuName.empty()) {
        QLabel *menuTag = new QLabel(QString::fromStdString(review.menuName));
        menuTag->setStyleSheet(
            "background:#f0f0f0; color:#444; font-size:12px;"
            "border-radius:4px; padding:2px 8px;");
        menuTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(menuTag);
    }

    // 리뷰 내용
    QLabel *contentLabel = new QLabel(QString::fromStdString(review.content));
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet("color: #555; font-size: 13px;");

    // 날짜
    QLabel *dateLabel = new QLabel(QString::fromStdString(review.createdAt));
    dateLabel->setStyleSheet("color: #aaa; font-size: 11px;");

    layout->addWidget(userLabel);
    layout->addWidget(contentLabel);
    layout->addWidget(dateLabel);

    // 구분선
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #eee;");
    layout->addWidget(line);

    // spacer 바로 앞(=0번 위치)에 삽입해서 위에서 아래로 쌓이게
    int insertPos = ui->reviewListLayout->count() - 1;
    ui->reviewListLayout->insertWidget(insertPos, item);
}