#include "readylist.h"
#include "ui_readylist.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

readylist::readylist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::readylist)
{
    ui->setupUi(this);
    // 초기화 시 기존 샘플 제거 로직 필요 시 추가
    ui->btn_back_list->hide();
    ui->lbl_title->hide();
    
// 2. 🚀 [범인 1] readylist 바깥쪽 메인 레이아웃의 여백과 '투명 박스 간격'을 완전히 0으로 압축!
    ui->mainLayout->setContentsMargins(0, 0, 0, 0); 
    ui->mainLayout->setSpacing(0);

    // 3. 🚀 [범인 2] 스크롤 영역 '안쪽(listContainer)'에 몰래 들어있던 기본 여백까지 0으로 제거!
    ui->listContainer->setContentsMargins(0, 0, 0, 0);
    ui->listContainer->setSpacing(0);

    ui->listContainer->setAlignment(Qt::AlignTop);
}

readylist::~readylist()
{
    delete ui;
}

void readylist::addOrderCard(const QString &orderId, const QString &storeName, const QString &status,
                             const QString &menuList, const QString &totalPrice)
{
    // 1. 메인 카드 프레임
    QFrame *card = new QFrame(this);
    card->setProperty("orderId", orderId);
    card->setStyleSheet("QFrame { background-color: white; border: 1px solid #eee; border-radius: 15px; }");
    QVBoxLayout *mainVLayout = new QVBoxLayout(card);
    mainVLayout->setContentsMargins(15, 15, 15, 15);
    mainVLayout->setSpacing(10);

    // --- 상단 영역 (아이콘, 가게명, 상태박스) ---
    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *icon = new QLabel(card);
    icon->setFixedSize(40, 40);
    icon->setStyleSheet("background-color: #f5f5f5; border-radius: 20px; border: none;"); // 대체 이미지

    QLabel *nameLabel = new QLabel(storeName, card);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 16px; border: none;");

    QLabel *statusBox = new QLabel(status, card);
    m_statusLabels[orderId] = statusBox;
    statusBox->setFixedSize(70, 25);
    statusBox->setAlignment(Qt::AlignCenter);
    statusBox->setStyleSheet("background-color: #eef9ff; color: #00b9ff; border: 1px solid #00b9ff; "
                             "border-radius: 5px; font-weight: bold; font-size: 11px;");

    topLayout->addWidget(icon);
    topLayout->addWidget(nameLabel);
    topLayout->addStretch();
    topLayout->addWidget(statusBox);

    // --- 중단 영역 (메뉴 목록) ---
    QLabel *menuLabel = new QLabel(menuList, card);
    menuLabel->setStyleSheet("color: #666; font-size: 13px; border: none;");
    menuLabel->setWordWrap(true); // 메뉴가 길면 줄바꿈

    // --- 하단 영역 (합계 금액, 버튼) ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    QLabel *priceLabel = new QLabel(QString("합계: %1").arg(totalPrice), card);
    priceLabel->setStyleSheet("font-weight: bold; font-size: 14px; border: none; color: #333;");

    QPushButton *detailBtn = new QPushButton("상세보기", card);
    detailBtn->setFixedSize(80, 30);
    detailBtn->setCursor(Qt::PointingHandCursor);
    detailBtn->setStyleSheet("QPushButton { background-color: #333; color: white; border-radius: 5px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #555; }");

    bottomLayout->addWidget(priceLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(detailBtn);

    // 메인 레이아웃에 합치기
    mainVLayout->addLayout(topLayout);
    mainVLayout->addWidget(menuLabel);
    // 구분선 (선택사항)
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #f5f5f5;");
    mainVLayout->addWidget(line);
    mainVLayout->addLayout(bottomLayout);

    // 최신 항목이 위로 오도록 추가 (Spacer 바로 위)
    ui->listContainer->insertWidget(ui->listContainer->count() - 1, card);

    // 상세보기 버튼에 시그널 연결
    connect(detailBtn, &QPushButton::clicked, this, [=]() {
        emit orderDetailRequested(orderId); 
    });
}

void readylist::updateCardStatus(const QString &orderId, int state) {
    if (!m_statusLabels.contains(orderId)) return;

    QLabel *label = m_statusLabels[orderId];
    QString statusText;

    // 인덱스에 따른 글자 설정
    switch(state) {
        case 0: statusText = "가게접수"; break;
        case 1: statusText = "조리중"; break;
        case 2: statusText = "조리완료"; break;
        case 3: statusText = "배달중"; break;
        default: statusText = "준비중"; break;
    }

    label->setText(statusText); // 화면의 글자가 실시간으로 바뀝니다!
}

void readylist::removeOrderCard(const QString &orderId)
{
    m_statusLabels.remove(orderId);

    // listContainer에서 해당 orderId 카드 찾아서 삭제
    for (int i = 0; i < ui->listContainer->count(); ++i) {
        QLayoutItem *layoutItem = ui->listContainer->itemAt(i);
        if (!layoutItem) continue;
        QWidget *w = layoutItem->widget();
        if (!w) continue;

        // 카드 안의 statusLabel이 m_statusLabels에서 삭제된 것과 같으면 해당 카드
        // orderId를 property로 저장하는 방식으로 찾기
        if (w->property("orderId").toString() == orderId) {
            ui->listContainer->removeItem(layoutItem);
            delete w;
            break;
        }
    }

    if (m_statusLabels.isEmpty()) {
        emit allCardsRemoved();
    }
}