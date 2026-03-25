#include "orderhistorywidget.h"
#include "ui_orderhistorywidget.h"
#include "UserSession.h"
#include <QDebug>
#include <QVBoxLayout>

// ============================================================
// 생성자
// ============================================================
OrderHistoryWidget::OrderHistoryWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderHistoryWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 탭 버튼 연결 ──
    connect(ui->tabHistory, &QPushButton::clicked,
            this, &OrderHistoryWidget::onTabHistoryClicked);
    connect(ui->tabPending, &QPushButton::clicked,
            this, &OrderHistoryWidget::onTabPendingClicked);

    // ── 과거 주문 내역 보기 버튼 ──
    connect(ui->btnGoHistory, &QPushButton::clicked,
            this, &OrderHistoryWidget::on_btnGoHistory_clicked);

    // ── TODO: 서버 응답 연결 ──
    // connect(m_network, &NetworkManager::onOrderHistoryReceived,
    //         this, &OrderHistoryWidget::onOrderHistoryReceived);
    // connect(m_network, &NetworkManager::onOrderListReceived,
    //         this, &OrderHistoryWidget::onOrderListReceived);


   m_readyList = new readylist(ui->pagePending);

   if (ui->pagePending->layout() != nullptr) {
        QVBoxLayout *pendingLayout = qobject_cast<QVBoxLayout*>(ui->pagePending->layout());
        if(pendingLayout) {
            pendingLayout->setContentsMargins(0, 0, 0, 0); 
            pendingLayout->setSpacing(0); // 🚀 이 줄을 추가해서 간격도 0으로 만드세요!
            
            pendingLayout->insertWidget(0, m_readyList); // 최상단에 꽂아 넣기
        }
    } else {
        QVBoxLayout *layout = new QVBoxLayout(ui->pagePending);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_readyList);
    }

    m_readyList->hide();
    
    // ── 초기 탭: 준비중 활성화 ──
    ui->tabStack->setCurrentWidget(ui->pagePending);
    setTabActive(ui->tabPending, ui->tabHistory);
}

void OrderHistoryWidget::addPendingOrder(const QString &orderId, const QString &storeName, const QString &menuSummary, int totalPrice)
{
   Q_UNUSED(orderId);

    // 실제 ui 파일에 존재하는 객체명으로 숨김 처리
    ui->pendingIcon->hide();
    ui->btnGoHistory->hide();
    
    m_readyList->show();
    
    // readylist에 카드 추가
    m_readyList->addOrderCard(storeName, "준비 중", menuSummary, QString::number(totalPrice) + "원");
}

OrderHistoryWidget::~OrderHistoryWidget() { delete ui; }

// ============================================================
// 화면 진입 시 서버 요청
// MainWindow에서 주문내역 탭 클릭 시 호출
// ============================================================
void OrderHistoryWidget::loadData()
{
    // 진입 시 항상 준비중 탭으로 초기화
    ui->tabStack->setCurrentWidget(ui->pagePending);
    setTabActive(ui->tabPending, ui->tabHistory);

    qDebug() << "[OrderHistoryWidget] loadData() - userId:"
             << UserSession::instance().userId;

    // TODO: REQ_ORDER_LIST(2050) 전송 — 준비중 주문 현황
    // TODO: REQ_ORDER_HISTORY(2080) 전송 — 과거 주문 내역
}

// ============================================================
// 탭 전환
// ============================================================
void OrderHistoryWidget::onTabHistoryClicked()
{
    ui->tabStack->setCurrentWidget(ui->pageHistory);
    setTabActive(ui->tabHistory, ui->tabPending);
}

void OrderHistoryWidget::onTabPendingClicked()
{
    ui->tabStack->setCurrentWidget(ui->pagePending);
    setTabActive(ui->tabPending, ui->tabHistory);
}

// ============================================================
// 준비중 탭 → "과거 주문 내역 보기" 버튼
// 과거 주문 내역 탭으로 전환
// ============================================================
void OrderHistoryWidget::on_btnGoHistory_clicked()
{
    onTabHistoryClicked();
}

// ============================================================
// 탭 스타일 헬퍼
// ============================================================
void OrderHistoryWidget::setTabActive(QPushButton *activeBtn, QPushButton *inactiveBtn)
{
    activeBtn->setStyleSheet(
        "QPushButton {"
        "  background:transparent; border:none;"
        "  border-bottom:2.5px solid #111111;"
        "  font-size:15px; font-weight:bold; color:#111111;"
        "  padding:0; margin:0;"
        "}"
    );
    inactiveBtn->setStyleSheet(
        "QPushButton {"
        "  background:transparent; border:none;"
        "  border-bottom:2.5px solid transparent;"
        "  font-size:15px; font-weight:bold; color:#aaaaaa;"
        "  padding:0; margin:0;"
        "}"
    );
}

// ============================================================
// 내비바 슬롯
// ============================================================
void OrderHistoryWidget::on_navHome_clicked()     { emit homeRequested(); }
void OrderHistoryWidget::on_navSearch_clicked()   { emit searchRequested(); }
void OrderHistoryWidget::on_navFavorite_clicked() { emit favoriteRequested(); }
void OrderHistoryWidget::on_navOrder_clicked()    {} // 현재 화면 — 아무 동작 없음
void OrderHistoryWidget::on_navMy_clicked()       { emit mypageRequested(); }
