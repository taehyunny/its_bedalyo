#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include "NetworkManager.h"
#include "cartsession.h"

// uic가 cartwidget.ui로부터 자동 생성하는 헤더
QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

// ============================================================
// CartWidget - 장바구니 / 결제 화면
//
// [ UI 구조 ]
// - cartwidget.ui : 정적 뼈대 (주소, 배달방법 섹션, 가격 라벨, 요청사항 등)
// - 코드           : 동적 생성 (배달 옵션 카드, 메뉴 카드)
//
// [ 흐름 ]
// 1. open() 호출 → UI 초기화 + REQ_CHECKOUT_INFO(2026) 전송
// 2. 서버에서 customerGrade, deliveryFee, minOrderAmount 수신
// 3. 배달방법 카드 동적 생성, 금액 라벨 업데이트
// 4. 결제하기 버튼 → REQ_ORDER_CREATE(2020) 전송
// ============================================================
class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~CartWidget();

    // MainWindow에서 장바구니 열 때 호출
    void open();

signals:
    void closeRequested();       // X 버튼 → 이전 화면으로
    void addMenuRequested();     // + 메뉴 추가 → 가게 상세로 복귀
    void addressEditRequested(); // 주소 수정 → 주소 관리 화면으로
    void orderSuccess();         // 주문 완료 → 홈으로

private slots:
    // [오류 수정] const QString& 로 통일 (헤더↔구현부 일치)
    void onCheckoutInfoReceived(int status, const QString &customerGrade,
                                int deliveryFee, int minOrderAmount);
    void onOrderCreateReceived(int status, const QString &message,
                               const QString &orderId);

    void on_btnClose_clicked();
    void on_btnAddMenu_clicked();
    void on_btnAddressEdit_clicked();
    void on_btnPay_clicked();
    void on_btnRequestToggle_clicked();

private:
    Ui::CartWidget *ui;        // .ui 파일로 생성된 위젯들 접근용
    NetworkManager *m_network;

    // ── 서버에서 받은 데이터 ──
    QString m_customerGrade;
    int     m_deliveryFee    = 0;
    int     m_minOrderAmount = 0;

    // ── 요청사항 펼침 여부 ──
    bool m_requestExpanded = true;

    // ── UI 업데이트 헬퍼 ──
    void updateAddress();           // 주소 라벨 갱신
    void updateDeliverySection();   // 배달 옵션 카드 동적 생성
    void rebuildMenuList();         // 메뉴 카드 목록 재빌드
    void updatePriceSection();      // 금액 라벨 갱신
    void updateBottomBar();         // 하단 바 갱신

    // ── 메뉴 카드 동적 생성 ──
    QWidget* makeMenuCard(const CartItemQt &item);

    // ── 계산 헬퍼 ──
    int  calcDeliveryFee() const;  // 와우면 0, 일반이면 m_deliveryFee
    int  calcDiscount()    const;  // 와우면 m_deliveryFee, 일반이면 0
    int  calcTotal()       const;
    bool isMinOrderMet()   const;
};
