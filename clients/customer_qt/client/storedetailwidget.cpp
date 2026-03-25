#include "storedetailwidget.h"
#include "ui_storedetailwidget.h"
#include "storeutils.h"
#include "cartsession.h"
#include <QFrame>
#include <QDebug>
#include <QMap>
#include <QScroller>

StoreDetailWidget::StoreDetailWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StoreDetailWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // CartBar 클릭 → cartRequested 시그널 발사
    connect(ui->cartBar, &CartBarWidget::cartRequested,
            this, &StoreDetailWidget::cartRequested);

    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->scrollArea_Reviews, QScroller::LeftMouseButtonGesture);

    loadDummyReviews();

    connect(m_network, &NetworkManager::onStoreDetailReceived,
            this, &StoreDetailWidget::onStoreDetailReceived);
}

StoreDetailWidget::~StoreDetailWidget() { delete ui; }

void StoreDetailWidget::loadStoreData(int storeId)
{
    ui->stackedWidget->setCurrentIndex(0);
    m_currentStoreId = storeId;
    qDebug() << "[StoreDetailWidget] 가게 상세 진입! 요청 ID:" << storeId;

    clearLayout(ui->menuListLayout);
    ui->lblStoreName->setText("메뉴를 불러오는 중...");
    m_network->sendStoreDetailRequest(storeId);
}

void StoreDetailWidget::onStoreDetailReceived(StoreDetailQt detail)
{
    if (detail.storeId != m_currentStoreId) return;

    if(m_lastStoreData) delete m_lastStoreData;
    m_lastStoreData = new StoreDetailQt(detail);

    ui->lblStoreName->setText(detail.storeName);

    double averageRating = 0.0;
    int reviewTotalCount = detail.reviews.size();

    if (reviewTotalCount > 0) {
        double sum = 0.0;
        for (const ReviewQt& r : detail.reviews) sum += r.rating;
        averageRating = sum / reviewTotalCount;
    } else {
        averageRating = detail.rating;
        reviewTotalCount = detail.reviewCount;
    }

    int starCount = qRound(averageRating);
    QString visualStars = "";
    for (int i = 0; i < 5; i++)
        visualStars += (i < starCount) ? "⭐" : "☆";

    ui->btnRating->setText(QString("%1 %2 (%3) >")
                           .arg(visualStars)
                           .arg(averageRating, 0, 'f', 1)
                           .arg(reviewTotalCount));

    ui->lblDeliveryStats->setText(QString("배달 %1 | 최소주문 %2원 | 배달비 %3")
                                  .arg(detail.deliveryTimeRange)
                                  .arg(detail.minOrderAmount)
                                  .arg(detail.deliveryFees));

    populatePhotoReviewBar(detail.reviews);

    QMap<QString, QList<MenuQt>> groupedMenus;
    for (const MenuQt& menu : detail.menus) {
        QString cat = menu.menuCategory.isEmpty() ? "기본 메뉴" : menu.menuCategory;
        groupedMenus[cat].append(menu);
    }

    for (auto it = groupedMenus.begin(); it != groupedMenus.end(); ++it) {
        QLabel* catLabel = new QLabel(it.key());
        catLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#111111; padding:15px 15px 5px 15px;");
        ui->menuListLayout->addWidget(catLabel);
        for (const MenuQt& menu : it.value())
            ui->menuListLayout->addWidget(makeMenuCard(menu));
    }

    if (!detail.reviews.isEmpty())
        renderReviews(detail.reviews);
}

QWidget* StoreDetailWidget::makeMenuCard(const MenuQt& menu)
{
    QWidget* cardContainer = new QWidget();
    cardContainer->setMinimumHeight(100);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    QGridLayout* stackLayout = new QGridLayout(cardContainer);
    stackLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* bgButton = new QPushButton();
    bgButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bgButton->setCursor(Qt::PointingHandCursor);

    if (menu.isSoldOut) {
        bgButton->setStyleSheet("QPushButton { background-color: #fafafa; border: none; border-bottom: 1px solid #eeeeee; }");
    } else {
        bgButton->setStyleSheet(
            "QPushButton { background-color: #ffffff; border: none; border-bottom: 1px solid #eeeeee; }"
            "QPushButton:pressed { background-color: #f5f5f5; }"
        );
    }

    QWidget* contentWidget = new QWidget();
    contentWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    contentWidget->setStyleSheet("background: transparent;");

    QHBoxLayout* mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    QLabel* nameLabel = new QLabel(menu.menuName);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #111111;");
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel* descLabel = new QLabel(menu.description);
    descLabel->setStyleSheet("font-size: 13px; color: #888888; padding-top: 3px; padding-bottom: 3px;");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    descLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QString priceText = menu.isSoldOut ? "품절" : StoreUtils::formatWon(menu.basePrice);
    QLabel* priceLabel = new QLabel(priceText);
    priceLabel->setStyleSheet(menu.isSoldOut
        ? "font-size: 15px; font-weight: bold; color: #d32f2f;"
        : "font-size: 15px; font-weight: bold; color: #111111;");
    priceLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descLabel);
    textLayout->addWidget(priceLabel);
    textLayout->addStretch();

    mainLayout->addLayout(textLayout);

    QLabel* imgLabel = new QLabel("사진");
    imgLabel->setFixedSize(80, 80);
    imgLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 8px; color:#999;");
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    mainLayout->addWidget(imgLabel);

    stackLayout->addWidget(bgButton, 0, 0);
    stackLayout->addWidget(contentWidget, 0, 0);

    connect(bgButton, &QPushButton::clicked, this, [this, menu]() {
        if (menu.isSoldOut) {
            qDebug() << "품절된 메뉴입니다:" << menu.menuName;
            return;
        }
        emit menuSelected(menu.menuId, menu.menuName, menu.basePrice);
    });

    return cardContainer;
}

void StoreDetailWidget::on_btnBack_clicked()
{
    if (ui->stackedWidget->currentIndex() > 0)
        ui->stackedWidget->setCurrentIndex(0);
    else
        emit backRequested();
}

void StoreDetailWidget::clearLayout(QLayout* layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}

void StoreDetailWidget::on_btnStoreInfoBack_clicked()  { ui->stackedWidget->setCurrentIndex(0); }
void StoreDetailWidget::on_btnRating_clicked()         { ui->stackedWidget->setCurrentIndex(2); }
void StoreDetailWidget::on_btnBackToMain_clicked()     { emit backRequested(); }

void StoreDetailWidget::renderReviews(const QList<ReviewQt>& reviews)
{
    clearLayout(ui->reviewListLayout);
    for (const ReviewQt& review : reviews)
        ui->reviewListLayout->addWidget(makeReviewCard(review));
    ui->reviewListLayout->addStretch();
}

QWidget* StoreDetailWidget::makeReviewCard(const ReviewQt& review)
{
    QWidget* card = new QWidget();
    card->setStyleSheet("background-color: white; border-bottom: 1px solid #eeeeee;");

    QVBoxLayout* mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(15, 20, 15, 20);
    mainLayout->setSpacing(10);

    QString maskedId = review.userId;
    if (maskedId.length() >= 1) maskedId = maskedId.left(1) + "**";
    QLabel* lblId = new QLabel(maskedId);
    lblId->setStyleSheet("font-size: 16px; font-weight: bold; color: #111;");
    mainLayout->addWidget(lblId);

    QHBoxLayout* ratingLayout = new QHBoxLayout();
    QString stars = "";
    for(int i=0; i<5; i++) stars += (i < review.rating) ? "⭐" : "☆";
    QLabel* lblStars = new QLabel(stars);
    lblStars->setStyleSheet("font-size: 14px;");
    QLabel* lblDate = new QLabel("|  " + review.createdAt);
    lblDate->setStyleSheet("font-size: 13px; color: #888888;");
    ratingLayout->addWidget(lblStars);
    ratingLayout->addWidget(lblDate);
    ratingLayout->addStretch();
    mainLayout->addLayout(ratingLayout);

    if (!review.orderedMenus.isEmpty()) {
        QLabel* lblMenu = new QLabel("주문메뉴  " + review.orderedMenus);
        lblMenu->setStyleSheet("font-size: 13px; color: #666666; font-weight: bold;");
        lblMenu->setWordWrap(true);
        mainLayout->addWidget(lblMenu);
    }

    QLabel* lblContent = new QLabel(review.comment);
    lblContent->setStyleSheet("font-size: 15px; color: #333333; line-height: 1.5; margin-top: 5px;");
    lblContent->setWordWrap(true);
    mainLayout->addWidget(lblContent);

    if (!review.ownerReply.isEmpty()) {
        QWidget* replyBox = new QWidget();
        replyBox->setStyleSheet("background-color: #f5f6f8; border-radius: 8px; margin-top: 10px;");
        QVBoxLayout* replyLayout = new QVBoxLayout(replyBox);
        replyLayout->setContentsMargins(15, 15, 15, 15);
        replyLayout->setSpacing(8);
        QHBoxLayout* replyHeader = new QHBoxLayout();
        QLabel* lblOwner = new QLabel("사장님");
        lblOwner->setStyleSheet("font-size: 14px; font-weight: bold; color: #111;");
        QLabel* lblReplyDate = new QLabel(review.createdAt);
        lblReplyDate->setStyleSheet("font-size: 13px; color: #888;");
        replyHeader->addWidget(lblOwner);
        replyHeader->addWidget(lblReplyDate);
        replyHeader->addStretch();
        replyLayout->addLayout(replyHeader);
        QLabel* lblReplyContent = new QLabel(review.ownerReply);
        lblReplyContent->setStyleSheet("font-size: 14px; color: #444; line-height: 1.5;");
        lblReplyContent->setWordWrap(true);
        replyLayout->addWidget(lblReplyContent);
        mainLayout->addWidget(replyBox);
    }

    return card;
}

void StoreDetailWidget::loadDummyReviews()
{
    QList<ReviewQt> dummyList;

    ReviewQt r1;
    r1.reviewId = 1;
    r1.userId = "김철수";
    r1.rating = 5;
    r1.createdAt = "3일 전";
    r1.orderedMenus = "황태콩나물국밥(공기밥 포함)";
    r1.comment = "너무 맛있게 잘 먹었습니다! 다음에도 꼭 여기서 시킬게요.";
    r1.ownerReply = "안녕하세요, 현대옥 광주화정점입니다. 리뷰 남겨 주셔서 감사합니다.";
    dummyList.append(r1);

    ReviewQt r2;
    r2.reviewId = 2;
    r2.userId = "김영희";
    r2.rating = 5;
    r2.createdAt = "4일 전";
    r2.orderedMenus = "얼큰돼지국밥(공기밥 포함) · 얼큰돼지국밥(공기밥 포함)";
    r2.comment = "남편이 어제 술먹고 해장한다고 지난번 시켰던곳 맛있었다고 주문요청해서 배달시켰네요~^^";
    dummyList.append(r2);

    renderReviews(dummyList);
}

void StoreDetailWidget::on_btnStoreInfo_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    populateStoreInfoPage();
}

void StoreDetailWidget::populateStoreInfoPage()
{
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->pageStoreInfo->layout());
    clearLayout(layout);

    QPushButton* btnBack = new QPushButton("← 뒤로가기");
    btnBack->setStyleSheet("border:none; font-size:16px; font-weight:bold; text-align:left; padding:15px; color:#333; background:transparent;");
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });
    layout->addWidget(btnBack);

    if (!m_lastStoreData) {
        layout->addWidget(new QLabel("매장 정보를 불러오고 있습니다...", this));
        layout->addStretch();
        return;
    }

    const StoreDetailQt& d = *m_lastStoreData;

    layout->addWidget(createInfoSection("📋", "매장정보"));
    layout->addWidget(createDataRow("영업시간", d.operatingHours.isEmpty() ? "정보 없음" : d.operatingHours));
    layout->addWidget(createDataRow("주소", d.storeAddress.isEmpty() ? "정보 없음" : d.storeAddress, true));
    layout->addSpacing(15);
    layout->addWidget(createInfoSection("💳", "결제정보"));
    layout->addWidget(createDataRow("최소주문금액", QString("%1원").arg(d.minOrderAmount)));
    layout->addSpacing(15);
    layout->addWidget(createInfoSection("🏢", "사업자정보"));
    layout->addWidget(createDataRow("상호명", d.storeName));
    layout->addStretch();
}

QWidget* StoreDetailWidget::createInfoSection(const QString& icon, const QString& title)
{
    QWidget* section = new QWidget();
    section->setStyleSheet("background-color: #f7f7f7;");
    QHBoxLayout* layout = new QHBoxLayout(section);
    layout->setContentsMargins(15, 10, 15, 10);
    layout->setSpacing(8);
    QLabel* lblIcon = new QLabel(icon);
    lblIcon->setStyleSheet("font-size: 16px;");
    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #111111;");
    layout->addWidget(lblIcon);
    layout->addWidget(lblTitle);
    layout->addStretch();
    return section;
}

QWidget* StoreDetailWidget::createDataRow(const QString& title, const QString& content, bool showMapButton)
{
    QWidget* row = new QWidget();
    row->setStyleSheet("background-color: white; border-bottom: 1px solid #eeeeee;");
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(10);
    QLabel* lblTitle = new QLabel(title);
    lblTitle->setFixedWidth(100);
    lblTitle->setStyleSheet("font-size: 14px; color: #555555;");
    QLabel* lblContent = new QLabel(content);
    lblContent->setStyleSheet("font-size: 14px; color: #111111;");
    lblContent->setWordWrap(true);
    layout->addWidget(lblTitle);
    layout->addWidget(lblContent, 1);
    if (showMapButton) {
        QPushButton* btnMap = new QPushButton("지도보기 >");
        btnMap->setStyleSheet("QPushButton { font-size: 12px; color: #ff3388; border: 1px solid #ff3388; border-radius: 4px; padding: 2px 8px; background: white; }"
                              "QPushButton:pressed { background-color: #fff0f5; }");
        btnMap->setCursor(Qt::PointingHandCursor);
        layout->addWidget(btnMap);
    }
    return row;
}

void StoreDetailWidget::populatePhotoReviewBar(const QList<ReviewQt>& reviews)
{
    QScrollArea* oldBar = ui->scrollContent->findChild<QScrollArea*>("photoReviewBar");
    if (oldBar) delete oldBar;
    if (reviews.isEmpty()) return;

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setObjectName("photoReviewBar");
    QScroller::grabGesture(scrollArea, QScroller::LeftMouseButtonGesture);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMaximumHeight(130);

    QWidget* scrollWidget = new QWidget();
    scrollWidget->setStyleSheet("background: transparent;");
    QHBoxLayout* hLayout = new QHBoxLayout(scrollWidget);
    hLayout->setContentsMargins(15, 0, 15, 10);
    hLayout->setSpacing(10);

    for (const ReviewQt& review : reviews) {
        for(int i = 0; i < 5; i++)
            hLayout->addWidget(createPhotoReviewCard(review));
        break;
    }

    scrollArea->setWidget(scrollWidget);
    ui->scrollLayout->insertWidget(2, scrollArea);
}

QWidget* StoreDetailWidget::createPhotoReviewCard(const ReviewQt& review)
{
    QWidget* card = new QWidget();
    card->setFixedSize(220, 120);
    card->setStyleSheet("background-color: #f7f7f7; border-radius: 8px; border: 1px solid #eeeeee;");

    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QLabel* lblPhoto = new QLabel("사진");
    lblPhoto->setFixedSize(60, 60);
    lblPhoto->setStyleSheet("background-color: #e0e0e0; border-radius: 4px; color: #999;");
    lblPhoto->setAlignment(Qt::AlignCenter);

    QWidget* infoWidget = new QWidget();
    infoWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* textLayout = new QVBoxLayout(infoWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);

    QString maskedId = review.userId;
    if(maskedId.length() > 1) maskedId = maskedId.left(1) + "**";
    QLabel* lblId = new QLabel(maskedId);
    lblId->setStyleSheet("font-size: 13px; font-weight: bold; color: #333;");

    QLabel* lblDate = new QLabel(review.createdAt);
    lblDate->setStyleSheet("font-size: 11px; color: #888;");

    QString stars = "";
    for(int i=0; i<5; i++) stars += (i < review.rating) ? "⭐" : "☆";
    QLabel* lblStars = new QLabel(stars);
    lblStars->setStyleSheet("font-size: 11px; color: #ffca28;");

    QLabel* lblComment = new QLabel(review.comment);
    lblComment->setStyleSheet("font-size: 12px; color: #555;");
    lblComment->setWordWrap(true);

    textLayout->addWidget(lblId);
    textLayout->addWidget(lblDate);
    textLayout->addWidget(lblStars);
    textLayout->addWidget(lblComment, 1);
    textLayout->addStretch();

    mainLayout->addWidget(lblPhoto);
    mainLayout->addWidget(infoWidget, 1);

    return card;
}

// CartBar 갱신 — CartSession 상태 보고 show/hide
void StoreDetailWidget::updateCartBar()
{
    ui->cartBar->updateCartUI();
}
