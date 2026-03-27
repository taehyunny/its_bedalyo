#pragma once
#include <QString>
#include <QList>
#include <QLocale>

// ============================================================
// CartItemQt - 장바구니 단일 메뉴 항목 (Qt용)
//
// 서버의 CartItem(C++ DTO)과 대응되는 Qt 타입
// 서버 연동 시 CartItem → CartItemQt 변환 필요
// ============================================================
struct CartItemQt {
    int     menuId;       // 메뉴 ID (서버 식별용)
    QString menuName;     // 메뉴명 (UI 표시용)
    int     quantity;     // 수량
    int     unitPrice;    // 단가 (원)
    // TODO: 옵션 목록 추가 예정 (서버팀과 DTO 확정 후)
    QList<int> optionIds;
};

// ============================================================
// CartSession - 장바구니 상태를 앱 전역에서 관리하는 싱글톤
//
// [ 특징 ]
// - 앱 실행 중에만 유지 (앱 종료 시 초기화 — 자동로그인 미구현으로 인한 설계)
// - 어느 화면에서든 CartSession::instance()로 접근 가능
// - 다른 가게 메뉴 담기 시도 시 → UI에서 경고 후 clear() 호출
//
// [ 사용법 ]
//   CartSession::instance().addItem(item);       // 메뉴 추가
//   CartSession::instance().totalCount();         // 총 수량
//   CartSession::instance().totalPrice();         // 총 금액
//   CartSession::instance().isEmpty();            // 비어있는지 확인
//   CartSession::instance().clear();              // 장바구니 비우기
//
// [ TODO: 서버 연동 후 추가할 것 ]
// - 로그인 응답(RES_LOGIN)에서 장바구니 데이터 받아서 loadFromServer() 호출
// - 메뉴 추가/수량변경 시 REQ_CART_ADD, REQ_CART_UPDATE 전송
// - 장바구니 비우기 시 REQ_CART_CLEAR 전송
// ============================================================
class CartSession
{
public:
    // ── 싱글톤 인스턴스 반환 ──
    static CartSession& instance()
    {
        static CartSession s;
        return s;
    }

    // ── 장바구니 소속 가게 정보 ──
    int     storeId   = -1;  // 현재 담긴 가게 ID (-1 = 비어있음)
    QString storeName;       // 가게명 (UI 표시용)

    // ── 메뉴 목록 ──
    QList<CartItemQt> items; // 담긴 메뉴 리스트

    // ============================================================
    // 메뉴 추가
    // - 같은 menuId가 이미 있으면 수량만 증가
    // - 다른 가게 메뉴라면 UI에서 먼저 경고 후 clear() 호출해야 함
    // ============================================================
    void addItem(const CartItemQt &item)
    {
        // 이미 담긴 동일 메뉴가 있으면 수량 증가
        for (CartItemQt &existing : items) {
            if (existing.menuId == item.menuId) {
                existing.quantity += item.quantity;
                return;
            }
        }
        // 새 메뉴 추가
        items.append(item);
    }

    // ============================================================
    // 수량 변경
    // quantity가 0 이하면 해당 메뉴 제거
    // ============================================================
    void updateQuantity(int menuId, int quantity)
    {
        if (quantity <= 0) {
            removeItem(menuId);
            return;
        }
        for (CartItemQt &item : items) {
            if (item.menuId == menuId) {
                item.quantity = quantity;
                return;
            }
        }
    }

    // ============================================================
    // 단일 메뉴 제거
    // ============================================================
    void removeItem(int menuId)
    {
        items.removeIf([menuId](const CartItemQt &i) {
            return i.menuId == menuId;
        });

        // 마지막 메뉴가 제거되면 가게 정보도 초기화
        if (items.isEmpty()) {
            storeId   = -1;
            storeName.clear();
        }
    }

    // ============================================================
    // 장바구니 전체 비우기 (다른 가게 진입 or 주문 완료 후 호출)
    // ============================================================
    void clear()
    {
        items.clear();
        storeId   = -1;
        storeName.clear();
    }

    // ── 상태 조회 ──

    // 장바구니가 비어있는지 여부
    bool isEmpty() const { return items.isEmpty(); }

    // 총 수량 (담긴 메뉴 수량의 합)
    int totalCount() const
    {
        int count = 0;
        for (const CartItemQt &item : items)
            count += item.quantity;
        return count;
    }

    // 총 금액 (단가 × 수량의 합)
    int totalPrice() const
    {
        int price = 0;
        for (const CartItemQt &item : items)
            price += item.unitPrice * item.quantity;
        return price;
    }

    // 총 금액 — 포맷된 문자열 (예: "24,000원")
    QString totalPriceStr() const
    {
        return QLocale(QLocale::Korean).toString(totalPrice()) + "원";
    }

    // 다른 가게 메뉴인지 확인 (UI에서 경고 팝업 띄우기 전 호출)
    bool isFromDifferentStore(int incomingStoreId) const
    {
        return !isEmpty() && storeId != incomingStoreId;
    }

private:
    // 외부 생성 불가 (싱글톤)
    CartSession() = default;
    CartSession(const CartSession&) = delete;
    CartSession& operator=(const CartSession&) = delete;
};
