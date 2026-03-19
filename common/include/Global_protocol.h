#pragma once // 중복 포함 방지 필수!

#include <string>
#include <cstdint>
#include "json.hpp"

using json = nlohmann::json;

// =========================================================================
// 1. 명령어 ID (PacketHeader에서 사용하기 위해 먼저 선언)
// 파트별로 번호 대역을 분리하여 각 클라이언트의 책임(R&R)을 명확히 함
// =========================================================================
enum class CmdID : uint16_t
{
    // ---------------------------------------------------------
    // [1000번대] 공통 및 시스템 (인증, 연결 유지)
    // ---------------------------------------------------------
    REQ_HEARTBEAT = 1000,               // 클라이언트 -> 서버: 실시간 연결 유지 확인
    RES_HEARTBEAT = 1001,               // 서버 -> 클라이언트: 연결 유지 응답

    REQ_LOGIN = 1010,                   // 공통: 로그인 요청 (고객, 사장님, 라이더 모두 사용)
    RES_LOGIN = 1011,                   // 공통: 로그인 응답
    REQ_SIGNUP = 1020,                  // 공통: 통합 회원가입 요청
    RES_SIGNUP = 1021,                  // 공통: 통합 회원가입 응답
    REQ_LOGOUT = 1030,                  // 공통: 로그아웃 요청
    RES_LOGOUT = 1031,                  // 공통: 로그아웃 응답
    REQ_AUTH_CHECK = 1040,              // 공통: 인증 상태 확인 요청
    RES_AUTH_CHECK = 1041,              // 공통: 인증 상태 확인 응답
    REQ_PROFILE_UPDATE = 1050,          // 공통: 프로필 업데이트 요청
    RES_PROFILE_UPDATE = 1051,          // 공통: 프로필 업데이트 응답

    // ---------------------------------------------------------
    // [2000번대] 고객(Customer) 파트
    // ---------------------------------------------------------
    REQ_STORE_LIST = 2000,              // 상점 목록 요청
    RES_STORE_LIST = 2001,              // 상점 목록 응답
    REQ_MENU_LIST = 2010,               // 특정 상점 메뉴 목록 요청
    RES_MENU_LIST = 2011,               // 특정 상점 메뉴 목록 응답
    REQ_ORDER_CREATE = 2020,            // 장바구니 결제 및 주문 생성 요청
    RES_ORDER_CREATE = 2021,            // 주문 생성 응답
    REQ_REVIEW_WRITE = 2030,            // 사진 첨부 리뷰 작성 요청
    RES_REVIEW_WRITE = 2031,            // 리뷰 작성 응답
    REQ_ADDRESS_SAVE = 2070,            //  (주소 저장 요청)
    RES_ADDRESS_SAVE = 2071,            //  (주소 저장 응답)
    REQ_ADDRESS_LIST = 2072,            //  (주소 목록 조회 요청)
    RES_ADDRESS_LIST = 2073,            //  (주소 목록 조회 응답)
    REQ_ADDRESS_DELETE = 2074,          //  (주소 삭제 요청)
    RES_ADDRESS_DELETE = 2075,          //  (주소 삭제 응답)

    // ---------------------------------------------------------
    // [3000번대] 사장님(Owner) 파트
    // ---------------------------------------------------------
    REQ_ORDER_ACCEPT = 3000,            // 주문 수락 및 조리 시작 요청 (예상 시간 포함)
    RES_ORDER_ACCEPT = 3001,            // 주문 수락 응답
    REQ_ORDER_REJECT = 3010,            // 주문 거절 요청 (품절 등 사유 포함)
    RES_ORDER_REJECT = 3011,            // 주문 거절 응답
    REQ_COOK_TIME_SET = 3020,           // 조리 시간 재설정 요청
    RES_COOK_TIME_SET = 3021,           // 조리 시간 설정 응답
    REQ_STORE_STATUS_SET = 3030,        // 영업 상태 수정 요청
    RES_STORE_STATUS_SET = 3031,        // 영업 상태 응답
    REQ_DELIVERY_POLICY_SET = 3040,     // 배달 반경 및 배달료 수정 요청
    RES_DELIVERY_POLICY_SET = 3041,     // 배달 반경 및 배달료 수정 응답
    REQ_MENU_SOLD_OUT  = 3050,          // 메뉴 품절 처리
    RES_MENU_SOLD_OUT = 3051,           // 메뉴 품절 응답
    REQ_MENU_EDIT = 3060,               // 메뉴 정보 편집 요청
    RES_MENU_EDIT = 3061,               // 메뉴 정보 편집 응답
    REQ_REVIEW_REPLY = 3070,            // 리뷰 답글 작성 요청
	RES_REVIEW_REPLY = 3071,            // 리뷰 답글 작성 응답
    REQ_SALES_STAT = 3090,              // 매출 및 정산 대시보드 요청
	RES_SALES_STAT = 3091,              // 매출 및 정산 대시보드 응답
    REQ_SETTLEMENT_INFO = 3100,         // 정산 내역 조회 요청
    RES_SETTLEMENT_INFO = 3101,         // 정산 내역 조회 응답
    REQ_COUPON_MANAGE = 3110,           // 쿠폰 발급 요청
    RES_COUPON_MANAGE = 3111,           // 쿠폰 발급 응답
    REQ_BLACKLIST_REQUEST = 3120,       // 차단 요청
    RES_BLACKLIST_REQUEST = 3121,       // 차단 응답
    // ---------------------------------------------------------
    // [4000번대] 라이더(Rider) 파트
    // ---------------------------------------------------------
    REQ_DELIVERY_ACCEPT = 4000,         // 서버가 뿌린 배달 콜 수락 요청
    RES_DELIVERY_ACCEPT = 4001,         // 배달 콜 수락 응답
    REQ_DELIVERY_COMPLETE = 4010,       // 현관 앞 도착 및 사진 인증 완료 요청
    RES_DELIVERY_COMPLETE = 4011,       // 사진 인증 완료 응답

    // ---------------------------------------------------------
    // [5000번대] 관리자(Admin) 파트
    // ---------------------------------------------------------
    RES_REQUEST_OK = 5000,              // 1대1 문의 요청 동의
    RES_REQUEST_NO = 5001,              // 1대1 문의 요청 비동의
    REQ_REFUND = 5010,                  // 환불 요청
    REQ_CANCEL = 5011,                  // 취소 요청
    RES_REFUND = 5012,                  // 환불 요청 응답
    RES_CANCEL = 5013,                  // 취소 요청 응답
    REQ_ORDER_LIST = 5020,              // 주문 내역 검색
    RES_ORDER_LIST = 5021,              // 주문 내역 응답
    REQ_RIDER_COUNT = 5030,             // 출근한 라이더가 몇명인지 요청
    RES_RIDER_COUNT = 5031,             // 출근한 라이더가 몇명인지 응답
    REQ_UNABLE_DELIVERY = 5040,         // 배달 불가 상태 알림  (서버 응답 : 브로드캐스팅)
    REQ_SURCHARGE = 5041,               // 할증 요청            (서버 응답 : 브로드캐스팅)
    RES_REVIEW_DELETE_OK = 5050,        // 악성 리뷰 삭제 동의
    RES_REVIEW_DELETE_NO = 5051,        // 악성 리뷰 삭제 비동의

    // ---------------------------------------------------------
    // [9000번대] 서버 푸시 알림 (Server Broadcast)
    // ---------------------------------------------------------
    NOTIFY_NEW_ORDER = 9000,            // 서버 -> 사장님: 새 주문 발생 알림
    NOTIFY_ORDER_STATE = 9010,          // 서버 -> 고객: 조리 시작, 배달 출발 등 상태 변경 알림
    NOTIFY_DELIVERY_CALL = 9020         // 서버 -> 라이더들: 주변 매장의 새 배달 콜 알림
};

// =========================================================================
// 2. 패킷 헤더 (1바이트 정렬 강제)
// =========================================================================
#pragma pack(push, 1)
struct PacketHeader
{                                // 모든 패킷의 맨 앞에 위치하여 데이터의 이정표 역할을 함
    uint16_t signature = 0x4543; // 기본값 할당 ('E','C' - Eats Connect)
    CmdID cmdId;                 // 명령어 ID (어떤 파트의 어떤 요청인지 식별)
    uint32_t bodySize;           // 뒤따라오는 JSON 바디의 실제 크기 (바이트 단위)
};
#pragma pack(pop) // [주의] DTO 선언 전에 반드시 pack 설정을 해제해야 함!
