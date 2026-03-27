#include "orderhistorywidget.h"
#include "ui_orderhistorywidget.h"
#include "UserSession.h"
#include "OrderHistoryCard.h"
#include "orderreceiptdialog.h"
#include "StoreUtils.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QScroller>
#include <QMessageBox>
#include <QPalette>
#include "PaymentDTO.h"

// ============================================================
// 생성자
// ============================================================
OrderHistoryWidget::OrderHistoryWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderHistoryWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // 스마트폰식 터치/드래그 스크롤(스와이프) 적용
    QScroller::grabGesture(ui->historyScrollArea, QScroller::LeftMouseButtonGesture);

    // ── 탭 버튼 연결 ──
    connect(ui->tabHistory, &QPushButton::clicked, this, &OrderHistoryWidget::onTabHistoryClicked);
    connect(ui->tabPending, &QPushButton::clicked, this, &OrderHistoryWidget::onTabPendingClicked);
    connect(ui->btnGoHistory, &QPushButton::clicked, this, &OrderHistoryWidget::on_btnGoHistory_clicked);

    // 🚀 1. 과거 주문 내역(2081) 수신 시 카드 그리기
    connect(m_network, &NetworkManager::onOrderHistoryReceived, this, [this](const ResOrderHistoryDTO &resDto){
        qDebug() << "[OrderHistory] status:" << resDto.status
                 << "count:" << resDto.historyList.size();
        // 기존 카드 전체 삭제
        QLayoutItem *child;
        while ((child = ui->historyListLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        if ((resDto.status == 200 || resDto.status == 0) && !resDto.historyList.empty()) {
            for (const auto& itemData : resDto.historyList) {
                OrderHistoryCard* card = new OrderHistoryCard(itemData, this);
                connect(card, &OrderHistoryCard::receiptRequested,
                        m_network, &NetworkManager::sendOrderDetailRequest);
                // ui->historyListLayout->insertWidget(0, card);
                ui->historyListLayout->addWidget(card);
            }
            // 빈 상태 숨기고 스크롤 영역 표시
            ui->historyEmptyWidget->hide();
            ui->historyScrollArea->show();
        } else {
            // 빈 상태 표시하고 스크롤 영역 숨기기
            ui->historyEmptyWidget->show();
            ui->historyScrollArea->hide();
        }
    });
    // 🚀 2. 영수증 상세 데이터(2087) 수신 시 팝업 띄우기
    connect(m_network, &NetworkManager::onOrderDetailReceived, this, [this](const ResOrderDetailDTO &resDto){
        if (resDto.status == 200 || resDto.status == 0) {
            QWidget *topParent = window();
            QWidget *overlay = new QWidget(topParent);
            overlay->setAutoFillBackground(true);
            QPalette pal = overlay->palette();
            pal.setColor(QPalette::Window, QColor(0, 0, 0, 150));
            overlay->setPalette(pal);
            overlay->setGeometry(0, 0, topParent->width(), topParent->height());
            overlay->show();
            overlay->raise();

            OrderReceiptDialog *dialog = new OrderReceiptDialog(resDto, topParent);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(dialog, &QDialog::finished, overlay, &QWidget::deleteLater);
            dialog->show();
            dialog->raise();
        }
    });

    // ── ReadyList 셋업 ──
    m_readyList = new readylist(ui->pagePending);

    QVBoxLayout *pendingLayout = qobject_cast<QVBoxLayout*>(ui->pagePending->layout());
    if (pendingLayout) {
        pendingLayout->insertWidget(0, m_readyList);
    }

    // ── 초기 탭: 준비중 활성화 ──
    ui->tabStack->setCurrentWidget(ui->pagePending);
    setTabActive(ui->tabPending, ui->tabHistory);

    m_readyList->hide();
    ui->historyEmptyWidget->show();
    ui->historyScrollArea->hide();

    // historyScrollArea가 네비바를 밀지 않도록 크기 정책 설정
    ui->historyScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_readyList, &readylist::allCardsRemoved, this, [this]() {
        m_readyList->hide();
        ui->pendingIcon->show();
        ui->btnGoHistory->show();
    });
}

void OrderHistoryWidget::addPendingOrder(const QString &orderId, const QString &storeName, const QString &menuSummary, int totalPrice)
{

    // 실제 ui 파일에 존재하는 객체명으로 숨김 처리
    ui->pendingIcon->hide();
    ui->btnGoHistory->hide();

    m_readyList->show();

    // readylist에 카드 추가
    m_readyList->addOrderCard(
        orderId,         // 👈 이게 빠지면 안 됩니다!
        storeName, 
        "가게접수",
        menuSummary,
        StoreUtils::formatWon(totalPrice)
    );

    m_pendingOrderData[orderId] = {storeName, menuSummary, QString::number(totalPrice)};
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
    m_network->sendOrderHistoryRequest(UserSession::instance().userId);
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
void OrderHistoryWidget::updateOrderState(const QString &orderId, int state) {
    if (m_readyList) {
        m_readyList->updateCardStatus(orderId, state); // 안쪽의 readylist에게 전달
    }
}

// 과거 주문 내역 탭으로 강제 이동하는 기능
void OrderHistoryWidget::showPastOrdersTab()
{
    if (ui->tabHistory) {
        // 버튼을 실제로 클릭한 효과를 줍니다
        ui->tabHistory->click();
    }
}

void OrderHistoryWidget::moveToHistory(const QString &orderId, const QString &statusText)
{
    // 1. 준비중에서 카드 삭제
    m_readyList->removeOrderCard(orderId);

    // 2. 저장해둔 데이터 꺼내기
    QStringList data = m_pendingOrderData.value(orderId, {"알 수 없는 가게", "", "0"});
    m_pendingOrderData.remove(orderId);

    // 3. 과거주문내역 카드 생성
    OrderHistoryItemDTO dto;
    dto.orderId     = orderId.toStdString();
    dto.storeName   = data[0].toStdString();
    dto.menuSummary = data[1].toStdString();
    dto.totalPrice  = data[2].toInt();
    dto.status      = 9; // 거절
    dto.createdAt   = "";
    dto.menuName    = statusText.toStdString(); // "배달거절" 표시용

    OrderHistoryCard *card = new OrderHistoryCard(dto, this);
    ui->historyEmptyWidget->hide();
    ui->historyScrollArea->show();
    ui->historyListLayout->insertWidget(0, card);
}