#pragma once

#include <QWidget>
#include <QPushButton>
#include "NetworkManager.h"
#include "readylist.h"
#include "OrderHistoryCard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OrderHistoryWidget; }
QT_END_NAMESPACE

// ============================================================
// OrderHistoryWidget - 주문 내역 화면
//
// [ 탭 구조 ]
// - 과거 주문 내역 (pageHistory): 주문 카드 리스트 (서버 연동 후 채움)
// - 준비중        (pagePending) : 클립보드 아이콘 + "과거 주문 내역 보기" 버튼
//   → 버튼 클릭 시 과거 주문 내역 탭으로 전환
//
// [ 프로토콜 ]
// REQ_ORDER_LIST    = 2050 / RES_ORDER_LIST    = 2051  (준비중 주문 현황)
// REQ_ORDER_HISTORY = 2080 / RES_ORDER_HISTORY = 2081  (과거 주문 내역)
// TODO: 서버팀과 주문 카드 DTO 확정 후 buildHistoryList() 구현
// ============================================================
class OrderHistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderHistoryWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~OrderHistoryWidget();

    // MainWindow에서 화면 진입 시 호출 → 서버에 데이터 요청
    void loadData();
    readylist* getReadyList() const { return m_readyList; }
    void addPendingOrder(const QString &orderId, const QString &storeName, const QString &menuSummary, int totalPrice); //추가
    void updateOrderState(const QString &orderId, int state);

    void showPastOrdersTab(); // 과거 주문 내역 탭을 강제로 보여주는 함수
    void addPastOrderCard(const PastOrderInfo& info);

signals:
    void homeRequested();
    void searchRequested();
    void favoriteRequested();
    void mypageRequested();

private slots:
    void onTabHistoryClicked();
    void onTabPendingClicked();
    void on_btnGoHistory_clicked();

    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

    // TODO: 서버 응답 슬롯
    // void onOrderHistoryReceived(...);
    // void onOrderListReceived(...);

private:
    Ui::OrderHistoryWidget *ui;
    NetworkManager         *m_network;
    readylist *m_readyList; // 주문 내역 목록 위젯 (추가)
    void setupPendingPage(); // 초기 레이아웃 설정 함수 (추가)

    // 탭 스타일 헬퍼
    void setTabActive(QPushButton *activeBtn, QPushButton *inactiveBtn);

    // TODO: 주문 카드 DTO 확정 후 구현
    // void buildHistoryList(const QList<OrderHistoryItem> &items);
    // QWidget* makeOrderCard(const OrderHistoryItem &item);
};
