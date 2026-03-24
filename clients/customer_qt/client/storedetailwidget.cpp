#include "storedetailwidget.h"
#include "ui_storedetailwidget.h"
#include "storeutils.h"
#include "cartsession.h"
#include <QFrame>
#include <QDebug>
#include <QMap>
#include <QScroller> // 스마트폰 터치처럼 드래그 스크롤

StoreDetailWidget::StoreDetailWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StoreDetailWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // 메인 화면(메뉴판) 위아래 스와이프 활성화
    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);
    
    // 리뷰 화면 위아래 스와이프 활성화
    QScroller::grabGesture(ui->scrollArea_Reviews, QScroller::LeftMouseButtonGesture);
    
    // UI가 뜨자마자 일단 더미 리뷰를 깔아둡니다.
    loadDummyReviews();

    // 서버 응답 신호 연결
    connect(m_network, &NetworkManager::onStoreDetailReceived,
            this, &StoreDetailWidget::onStoreDetailReceived);
}

StoreDetailWidget::~StoreDetailWidget() { delete ui; }

// ============================================================
// 진입점: MainWindow에서 넘어올 때 무조건 실행됨!
// ============================================================
void StoreDetailWidget::loadStoreData(int storeId)
{
    // 🚀 [수정됨] 3페이지 진입 시 강제로 0번(메뉴) 화면부터 켜기!
    ui->stackedWidget->setCurrentIndex(0);


    m_currentStoreId = storeId;
    qDebug() << "[StoreDetailWidget] 가게 상세 진입! 요청 ID:" << storeId;

    // 1. 기존 메뉴들 청소
    clearLayout(ui->menuListLayout);
    ui->lblStoreName->setText("메뉴를 불러오는 중...");

    // 원래 코드: 서버에 요청 (잠시 주석 처리)
    m_network->sendStoreDetailRequest(storeId);

    // 더미 코드: 가짜 데이터 직접 만들어서 UI 함수 강제 실행(테스트용)
    // StoreDetailQt fakeData;
    // fakeData.storeId = storeId;
    // fakeData.storeName = "🔥 테스트용 불향 쭈꾸미 (가짜데이터)";
    
    // MenuQt menu1;
    // menu1.menuName = "쭈꾸미 삼겹살 2인분";
    // menu1.description = "매콤달콤 끝판왕!";
    // menu1.basePrice = 28000;
    // menu1.menuCategory = "메인 메뉴";
    
    // MenuQt menu2;
    // menu2.menuName = "날치알 볶음밥";
    // menu2.description = "K-디저트는 못 참지";
    // menu2.basePrice = 4000;
    // menu2.menuCategory = "사이드 메뉴";
    
    // fakeData.menus.append(menu1);
    // fakeData.menus.append(menu2);

    // // 가짜 데이터를 넣어서 화면 그리는 함수를 직접 호출!
    // onStoreDetailReceived(fakeData);
}

// ============================================================
// 서버에서 '종합 선물 세트'를 받았을 때 실행됨!
// ============================================================
void StoreDetailWidget::onStoreDetailReceived(StoreDetailQt detail)
{
    if (detail.storeId != m_currentStoreId) return;

    // 나중에 정보 탭을 열 때 쓰기 위해 현재 데이터 복사해두기
    if(m_lastStoreData) delete m_lastStoreData;
    m_lastStoreData = new StoreDetailQt(detail);

    // 1. 가게 이름 세팅
    ui->lblStoreName->setText(detail.storeName);

    // ------------------------------------------------------------
    // 리뷰 데이터를 바탕으로 평균 별점과 리뷰 개수 직접 계산!
    // ------------------------------------------------------------
    double averageRating = 0.0;
    int reviewTotalCount = detail.reviews.size(); // 실제 넘어온 리뷰 리스트의 개수
    
    if (reviewTotalCount > 0) {
        double sum = 0.0;
        for (const ReviewQt& r : detail.reviews) {
            sum += r.rating; // 각 리뷰의 별점을 모두 더함
        }
        averageRating = sum / reviewTotalCount; // 총합 / 리뷰 개수 = 평균 평점
    } else {
        // 만약 서버에서 아직 리뷰 리스트를 안 보냈다면, 서버가 준 기본값(StoreDataDTO) 사용
        averageRating = detail.rating;
        reviewTotalCount = detail.reviewCount;
    }

    // 2. 계산된 평균 점수로 별(⭐) 시각화 만들기 (예: 4.5점 -> ⭐⭐⭐⭐☆)
    int starCount = qRound(averageRating); // 반올림해서 꽉 찬 별의 개수를 정함
    QString visualStars = "";
    for (int i = 0; i < 5; i++) {
        if (i < starCount) {
            visualStars += "⭐"; // 점수만큼 꽉 찬 별
        } else {
            visualStars += "☆";  // 나머지는 빈 별
        }
    }

    // 3. 버튼 텍스트 최종 업데이트 (예: "⭐⭐⭐⭐☆ 4.5 (3) >")
    ui->btnRating->setText(QString("%1 %2 (%3) >")
                           .arg(visualStars)
                           .arg(averageRating, 0, 'f', 1) // 소수점 1자리까지만 표시 (예: 4.5)
                           .arg(reviewTotalCount));       // 계산된 총 리뷰 수
    // ------------------------------------------------------------
    
    // 배달 정보 세팅
    ui->lblDeliveryStats->setText(QString("배달 %1 | 최소주문 %2원 | 배달비 %3")
                                  .arg(detail.deliveryTimeRange).arg(detail.minOrderAmount).arg(detail.deliveryFees));

    // 가로 스크롤 리뷰 바 생성 명령
    populatePhotoReviewBar(detail.reviews);

    // 2. 메뉴판 쫙 깔아주기
    QMap<QString, QList<MenuQt>> groupedMenus;
    for (const MenuQt& menu : detail.menus) {
        QString cat = menu.menuCategory.isEmpty() ? "기본 메뉴" : menu.menuCategory;
        groupedMenus[cat].append(menu);
    }

    for (auto it = groupedMenus.begin(); it != groupedMenus.end(); ++it) {
        QLabel* catLabel = new QLabel(it.key());
        catLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#111111; padding:15px 15px 5px 15px;");
        ui->menuListLayout->addWidget(catLabel);

        for (const MenuQt& menu : it.value()) {
            ui->menuListLayout->addWidget(makeMenuCard(menu));
        }
    }
    // 서버에서 '진짜 리뷰 데이터'가 오면 더미를 지우고 진짜를 그립니다!
    if (!detail.reviews.isEmpty()) {
        renderReviews(detail.reviews); 
    }
}

// ============================================================
// 개별 메뉴 카드 만들기
// ============================================================
QWidget* StoreDetailWidget::makeMenuCard(const MenuQt& menu) 
{
    // 1. 전체를 담을 빈 컨테이너 (여기에 레이아웃 에러가 나지 않도록 세팅)
    QWidget* cardContainer = new QWidget();
    cardContainer->setMinimumHeight(100);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    // QGridLayout을 사용하면 여러 위젯을 같은 칸(0, 0)에 겹쳐서 올릴 수 있습니다
    QGridLayout* stackLayout = new QGridLayout(cardContainer);
    stackLayout->setContentsMargins(0, 0, 0, 0);

    // 2. 바닥(1층)에 깔릴 거대한 배경 버튼
    QPushButton* bgButton = new QPushButton();
    bgButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bgButton->setCursor(Qt::PointingHandCursor);
    
    // 품절 여부에 따른 스타일
    if (menu.isSoldOut) {
        bgButton->setStyleSheet("QPushButton { background-color: #fafafa; border: none; border-bottom: 1px solid #eeeeee; }");
    } else {
        bgButton->setStyleSheet(
            "QPushButton { background-color: #ffffff; border: none; border-bottom: 1px solid #eeeeee; }"
            "QPushButton:pressed { background-color: #f5f5f5; }"
        );
    }

    // 3. 위(2층)에 올라갈 텍스트와 사진 위젯
    QWidget* contentWidget = new QWidget();
    // 마우스 클릭이 뒤에 있는 bgButton으로 뚫고 지나가게 함
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
    priceLabel->setStyleSheet(menu.isSoldOut ? "font-size: 15px; font-weight: bold; color: #d32f2f;" : "font-size: 15px; font-weight: bold; color: #111111;");
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

    // 4. 컨테이너에 버튼과 내용을 겹쳐서 추가 (0행, 0열)
    stackLayout->addWidget(bgButton, 0, 0);     // 1층: 배경 버튼
    stackLayout->addWidget(contentWidget, 0, 0); // 2층: 내용 (투명)

    // 5. 클릭 이벤트는 1층에 깔린 bgButton에서 받아서 처리!
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
    // 현재 화면이 리뷰(2)나 정보(1)라면 메뉴(0)로 돌아오고, 
    // 메뉴(0) 화면이라면 아예 가게 밖(목록)으로 나갑니다.
    if (ui->stackedWidget->currentIndex() > 0) {
        ui->stackedWidget->setCurrentIndex(0); 
    } else {
        emit backRequested(); 
    }
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

void StoreDetailWidget::on_btnStoreInfoBack_clicked() {
    ui->stackedWidget->setCurrentIndex(0); 
}

// 별점 버튼 클릭 시 -> 2번 인덱스(리뷰 화면)로 이동!
void StoreDetailWidget::on_btnRating_clicked() 
{
    ui->stackedWidget->setCurrentIndex(2); 
}

// 누락되었던 메인 화면 뒤로가기 함수 추가!
void StoreDetailWidget::on_btnBackToMain_clicked() 
{
    // 메인 화면에서 뒤로가기를 누르면 MainWindow 대장님께 화면을 꺼달라고 신호 발사
    emit backRequested(); 
}

// ============================================================
// 1. 리뷰 목록 전체 렌더링 함수
// ============================================================
void StoreDetailWidget::renderReviews(const QList<ReviewQt>& reviews)
{
    // 기존에 있던 리뷰(더미 포함) 싹 지우기
    clearLayout(ui->reviewListLayout);

    // 리뷰 개수만큼 카드 만들어서 꽂아넣기
    for (const ReviewQt& review : reviews) {
        ui->reviewListLayout->addWidget(makeReviewCard(review));
    }
    
    // 스크롤 위쪽으로 정렬되도록 맨 밑에 빈 공간(스프링) 추가
    ui->reviewListLayout->addStretch();
}

// ============================================================
// 2. 개별 리뷰 카드 UI 생성
// ============================================================
QWidget* StoreDetailWidget::makeReviewCard(const ReviewQt& review)
{
    QWidget* card = new QWidget();
    card->setStyleSheet("background-color: white; border-bottom: 1px solid #eeeeee;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(15, 20, 15, 20);
    mainLayout->setSpacing(10);

    // 1. 아이디 (김** 형태로 마스킹)
    QString maskedId = review.userId;
    if (maskedId.length() >= 1) {
        maskedId = maskedId.left(1) + "**"; // 첫 글자만 보이고 나머지 ** (캡쳐 반영)
    }
    QLabel* lblId = new QLabel(maskedId);
    lblId->setStyleSheet("font-size: 16px; font-weight: bold; color: #111;");
    mainLayout->addWidget(lblId);

    // 2. 별점(⭐)과 날짜
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

    // 3. 주문 메뉴 (연한 회색 바탕)
    if (!review.orderedMenus.isEmpty()) {
        QLabel* lblMenu = new QLabel("주문메뉴  " + review.orderedMenus);
        lblMenu->setStyleSheet("font-size: 13px; color: #666666; font-weight: bold;");
        lblMenu->setWordWrap(true);
        mainLayout->addWidget(lblMenu);
    }

    // 4. 리뷰 내용 본문
    QLabel* lblContent = new QLabel(review.comment);
    lblContent->setStyleSheet("font-size: 15px; color: #333333; line-height: 1.5; margin-top: 5px;");
    lblContent->setWordWrap(true);
    mainLayout->addWidget(lblContent);

    // 5. 사장님 답글 박스 (캡쳐 화면의 회색 둥근 박스)
    if (!review.ownerReply.isEmpty()) {
        QWidget* replyBox = new QWidget();
        replyBox->setStyleSheet("background-color: #f5f6f8; border-radius: 8px; margin-top: 10px;");
        
        QVBoxLayout* replyLayout = new QVBoxLayout(replyBox);
        replyLayout->setContentsMargins(15, 15, 15, 15);
        replyLayout->setSpacing(8);

        // 사장님 타이틀 + 날짜
        QHBoxLayout* replyHeader = new QHBoxLayout();
        QLabel* lblOwner = new QLabel("사장님");
        lblOwner->setStyleSheet("font-size: 14px; font-weight: bold; color: #111;");
        QLabel* lblReplyDate = new QLabel(review.createdAt); // 임시로 같은 날짜 사용
        lblReplyDate->setStyleSheet("font-size: 13px; color: #888;");
        
        replyHeader->addWidget(lblOwner);
        replyHeader->addWidget(lblReplyDate);
        replyHeader->addStretch();
        replyLayout->addLayout(replyHeader);

        // 사장님 답글 내용
        QLabel* lblReplyContent = new QLabel(review.ownerReply);
        lblReplyContent->setStyleSheet("font-size: 14px; color: #444; line-height: 1.5;");
        lblReplyContent->setWordWrap(true);
        replyLayout->addWidget(lblReplyContent);

        mainLayout->addWidget(replyBox);
    }

    return card;
}

// ============================================================
// 3. 더미 데이터 로드 (서버 데이터 오기 전 보여줄 테스트용)
// ============================================================
void StoreDetailWidget::loadDummyReviews()
{
    QList<ReviewQt> dummyList;

    // 캡쳐 화면 첫 번째 리뷰 (답글 있음)
    ReviewQt r1;
    r1.reviewId = 1;
    r1.userId = "김철수"; // 김** 로 자동 변환됨
    r1.rating = 5;
    r1.createdAt = "3일 전";
    r1.orderedMenus = "황태콩나물국밥(공기밥 포함)";
    r1.comment = "너무 맛있게 잘 먹었습니다! 다음에도 꼭 여기서 시킬게요.";
    r1.ownerReply = "안녕하세요, 현대옥 광주화정점입니다. 리뷰 남겨 주셔서 감사합니다. 저희 가게를 좋게 봐주셔서 정말 힘이 나네요. 앞으로도 정성껏 준비하겠습니다. 다음에 또 주문 부탁드려요! 😊👍";
    dummyList.append(r1);

    // 캡쳐 화면 두 번째 리뷰 (답글 없음)
    ReviewQt r2;
    r2.reviewId = 2;
    r2.userId = "김영희"; 
    r2.rating = 5;
    r2.createdAt = "4일 전";
    r2.orderedMenus = "얼큰돼지국밥(공기밥 포함) · 얼큰돼지국밥(공기밥 포함)";
    r2.comment = "남편이 어제 술먹고 해장한다고 지난번 시켰던곳 맛있었다고 주문요청해서 배달시켰네요~^^";
    // r2.ownerReply 는 비워둠 (답글 박스 안 생김)
    dummyList.append(r2);

    // UI에 그리기
    renderReviews(dummyList);
}

// ============================================================
// 🚀 상단 '정보' 탭 클릭 시 실행되는 슬롯 (버튼 connect 필요)
// ============================================================
void StoreDetailWidget::on_btnStoreInfo_clicked()
{
    // 스택 위젯을 1번(정보 페이지)로 변경
    ui->stackedWidget->setCurrentIndex(1); 
    
    // UI 그리기 함수 호출
    populateStoreInfoPage();
}

// ============================================================
// 더미 없이 '있는 데이터만' 활용하여 정보 페이지 그리기
// ============================================================
void StoreDetailWidget::populateStoreInfoPage()
{
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->pageStoreInfo->layout());
    clearLayout(layout); // 여기서 기존에 있던 뒤로가기 버튼까지 싹 지워집니다

    // 🚀 [해결] 1. 코드로 '뒤로가기' 버튼을 무적 상태로 부활시킵니다!
    QPushButton* btnBack = new QPushButton("← 뒤로가기");
    btnBack->setStyleSheet("border:none; font-size:16px; font-weight:bold; text-align:left; padding:15px; color:#333; background:transparent;");
    btnBack->setCursor(Qt::PointingHandCursor);
    
    // 버튼을 누르면 0번(메뉴판) 화면으로 돌아가게 연결!
    connect(btnBack, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0); 
    });
    layout->addWidget(btnBack); // 맨 꼭대기에 추가

    // 데이터가 없으면 안내 문구 띄우기
    if (!m_lastStoreData) {
        layout->addWidget(new QLabel("매장 정보를 불러오고 있습니다...", this));
        layout->addStretch();
        return;
    }

    const StoreDetailQt& d = *m_lastStoreData;

    // --- 1. 매장정보 섹션 ---
    layout->addWidget(createInfoSection("📋", "매장정보"));
    
    QString opHours = d.operatingHours.isEmpty() ? "정보 없음" : d.operatingHours;
    layout->addWidget(createDataRow("영업시간", opHours));
    
    QString addr = d.storeAddress.isEmpty() ? "정보 없음" : d.storeAddress;
    layout->addWidget(createDataRow("주소", addr, true)); 

    layout->addSpacing(15);

    // --- 2. 결제정보 섹션 ---
    layout->addWidget(createInfoSection("💳", "결제정보"));
    
    QString minAmt = QString("%1원").arg(d.minOrderAmount);
    layout->addWidget(createDataRow("최소주문금액", minAmt));

    layout->addSpacing(15);

    // --- 3. 사업자정보 섹션 ---
    layout->addWidget(createInfoSection("🏢", "사업자정보"));
    
    layout->addWidget(createDataRow("상호명", d.storeName));

    layout->addStretch();
}

// ============================================================
// 🚀 회색 타이틀 섹션(예: 📋 매장정보) 헬퍼 함수
// ============================================================
QWidget* StoreDetailWidget::createInfoSection(const QString& icon, const QString& title)
{
    QWidget* section = new QWidget();
    section->setStyleSheet("background-color: #f7f7f7;"); // 연한 회색 배경
    
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

// ============================================================
// 흰색 데이터 한 줄(예: 영업시간  09:00~21:00) 헬퍼 함수
// ============================================================
QWidget* StoreDetailWidget::createDataRow(const QString& title, const QString& content, bool showMapButton)
{
    QWidget* row = new QWidget();
    row->setStyleSheet("background-color: white; border-bottom: 1px solid #eeeeee;"); 
    
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(10);

    // 제목 (왼쪽 정렬, 고정 너비)
    QLabel* lblTitle = new QLabel(title);
    lblTitle->setFixedWidth(100);
    lblTitle->setStyleSheet("font-size: 14px; color: #555555;"); 
    
    // 내용
    QLabel* lblContent = new QLabel(content);
    lblContent->setStyleSheet("font-size: 14px; color: #111111;"); 
    lblContent->setWordWrap(true); 
    
    layout->addWidget(lblTitle);
    layout->addWidget(lblContent, 1); // 내용이 남은 공간을 다 차지하도록

    // '지도보기' 버튼 (주소 란에만 표시)
    if (showMapButton) {
        QPushButton* btnMap = new QPushButton("지도보기 >");
        btnMap->setStyleSheet("QPushButton { font-size: 12px; color: #ff3388; border: 1px solid #ff3388; border-radius: 4px; padding: 2px 8px; background: white; } "
                              "QPushButton:pressed { background-color: #fff0f5; }");
        btnMap->setCursor(Qt::PointingHandCursor);
        layout->addWidget(btnMap);
    }
    
    return row;
}

// ============================================================
// 코드로 직접 만드는 가로 스크롤 리뷰 바
// ============================================================
void StoreDetailWidget::populatePhotoReviewBar(const QList<ReviewQt>& reviews)
{
    // 0. 기존에 만든 리뷰 바가 있다면 청소 (다른 가게 클릭 시 중복 생성 방지)
    QScrollArea* oldBar = ui->scrollContent->findChild<QScrollArea*>("photoReviewBar");
    if (oldBar) {
        delete oldBar;
    }

    // 1. 리뷰가 하나도 없으면 그리지 않고 즉시 종료
    if (reviews.isEmpty()) return;

    // 2. 코드로 직접 가로 스크롤 상자(QScrollArea) 만들기
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setObjectName("photoReviewBar"); // 이름표 달아주기 (나중에 찾아서 지울 수 있게)

    // 가로 리뷰 바 좌우 스와이프 활성화!
    QScroller::grabGesture(scrollArea, QScroller::LeftMouseButtonGesture);
    
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 세로 끔
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 가로 켬
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame); // 테두리 없음
    scrollArea->setMaximumHeight(130); // 캡쳐 화면 크기로 고정

    // 3. 스크롤 상자 안에 들어갈 알맹이 위젯과 가로 레이아웃 설정
    QWidget* scrollWidget = new QWidget();
    scrollWidget->setStyleSheet("background: transparent;");
    QHBoxLayout* hLayout = new QHBoxLayout(scrollWidget);
    // 왼쪽/오른쪽 여백을 15px 정도 줘서 화면 끝에 너무 딱 붙지 않게 예쁘게 처리
    hLayout->setContentsMargins(15, 0, 15, 10); 
    hLayout->setSpacing(10);

    // 4. 리뷰 카드 만들어서 알맹이 가로 레이아웃에 꽂아 넣기
    int count = 0;
    for (const ReviewQt& review : reviews) {
        // [테스트용 복사 마법] 리뷰가 1개여도 가로 스크롤을 보기 위해 5번 강제 반복!
        for(int i = 0; i < 5; i++) {
            QWidget* card = createPhotoReviewCard(review);
            hLayout->addWidget(card);
        }
        break; // 5개 만들었으니 바로 반복문 종료
    }

    // 5. 완성된 알맹이를 스크롤 상자에 집어넣기
    scrollArea->setWidget(scrollWidget);

    // 6. [위치 수정] '메뉴' 글씨 위, '매장 정보' 아래로 정확히 꽂아넣기!
    // ui->scrollLayout은 전체 화면을 담는 큰 상자입니다.
    // 인덱스 0: 가게 사진, 인덱스 1: 정보박스(최소주문 등), 인덱스 2: 메뉴 탭("메뉴")
    // 따라서 2번에 insert 하면 정보박스와 메뉴 탭 사이에 완벽하게 끼어들어갑니다!
    ui->scrollLayout->insertWidget(2, scrollArea);
}

// ============================================================
// 가로 스크롤 안에 들어갈 개별 리뷰 사진 카드 만들기
// ============================================================
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

void StoreDetailWidget::updateCartBar() {
    // 이제 이 함수는 아무 일도 하지 않습니다.
}