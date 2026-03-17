#pragma once // 중복 포함 방지 필수!

#include <string>
#include <cstdint>
#include "json.hpp"

using json = nlohmann::json;

// =========================================================================
// 1. 명령어 ID (PacketHeader에서 사용하기 위해 먼저 선언)
// 파트별로 번호 대역을 분리하여 각 클라이언트의 책임(R&R)을 명확히 함
// =========================================================================
enum class CmdID : uint16_t { 
    // ---------------------------------------------------------
    // [1000번대] 공통 및 시스템 (인증, 연결 유지)
    // ---------------------------------------------------------
    REQ_HEARTBEAT = 1000,   // 클라이언트 -> 서버: 실시간 연결 유지 확인
    RES_HEARTBEAT = 1001,   // 서버 -> 클라이언트: 연결 유지 응답

    REQ_LOGIN = 1010,       // 공통: 로그인 요청 (고객, 사장님, 라이더 모두 사용)
    RES_LOGIN = 1011,       // 공통: 로그인 응답
    REQ_SIGNUP = 1020,      // 공통: 통합 회원가입 요청
    RES_SIGNUP = 1021,      // 공통: 통합 회원가입 응답
    REQ_LOGOUT = 1030,      // 공통: 로그아웃 요청
    RES_LOGOUT = 1031,      // 공통: 로그아웃 응답
    REQ_AUTH_CHECK = 1040,  // 공통: 인증 상태 확인 요청
    RES_AUTH_CHECK = 1041,  // 공통: 인증 상태 확인 응답
    REQ_PROFILE_UPDATE = 1050, // 공통: 프로필 업데이트 요청
    RES_PROFILE_UPDATE = 1051, // 공통: 프로필 업데이트 응답


    // ---------------------------------------------------------
    // [2000번대] 고객(Customer) 파트
    // ---------------------------------------------------------
    REQ_STORE_LIST = 2000,  // 상점 목록 요청
    RES_STORE_LIST = 2001,  // 상점 목록 응답
    REQ_MENU_LIST = 2010,   // 특정 상점 메뉴 목록 요청
    RES_MENU_LIST = 2011,   // 특정 상점 메뉴 목록 응답
    REQ_ORDER_CREATE = 2020,// 장바구니 결제 및 주문 생성 요청
    RES_ORDER_CREATE = 2021,// 주문 생성 응답
    REQ_REVIEW_WRITE = 2030,// 사진 첨부 리뷰 작성 요청
    RES_REVIEW_WRITE = 2031,// 리뷰 작성 응답

    // ---------------------------------------------------------
    // [3000번대] 사장님(Owner) 파트
    // ---------------------------------------------------------
    REQ_ORDER_ACCEPT = 3000,// 주문 수락 및 조리 시작 요청 (예상 시간 포함)
    RES_ORDER_ACCEPT = 3001,// 주문 수락 응답
    REQ_ORDER_REJECT = 3010,// 주문 거절 요청 (품절 등 사유 포함)
    RES_ORDER_REJECT = 3011,// 주문 거절 응답
    REQ_CALL_RIDER = 3020,  // 조리 완료 임박, 배달 기사 호출 요청
    RES_CALL_RIDER = 3021,  // 배달 기사 호출 응답

    // ---------------------------------------------------------
    // [4000번대] 라이더(Rider) 파트
    // ---------------------------------------------------------
    REQ_DELIVERY_ACCEPT = 4000,   // 서버가 뿌린 배달 콜 수락 요청
    RES_DELIVERY_ACCEPT = 4001,   // 배달 콜 수락 응답
    REQ_DELIVERY_COMPLETE = 4010, // 현관 앞 도착 및 사진 인증 완료 요청
    RES_DELIVERY_COMPLETE = 4011, // 사진 인증 완료 응답

    // ---------------------------------------------------------
    // [5000번대] 관리자(Admin) 파트
    // ---------------------------------------------------------
    REQ_MONITOR_SYNC = 5000,  // 전체 상점 및 주문 상태 동기화 요청
    RES_MONITOR_SYNC = 5001,  // 동기화 응답
    REQ_FORCE_CANCEL = 5010,  // 관리자 권한 주문 강제 취소 요청
    RES_FORCE_CANCEL = 5011,  // 주문 강제 취소 응답

    // ---------------------------------------------------------
    // [9000번대] 서버 푸시 알림 (Server Broadcast)
    // ---------------------------------------------------------
    NOTIFY_NEW_ORDER = 9000,     // 서버 -> 사장님: 새 주문 발생 알림
    NOTIFY_ORDER_STATE = 9010,   // 서버 -> 고객: 조리 시작, 배달 출발 등 상태 변경 알림
    NOTIFY_DELIVERY_CALL = 9020  // 서버 -> 라이더들: 주변 매장의 새 배달 콜 알림
};

// =========================================================================
// 2. 패킷 헤더 (1바이트 정렬 강제)
// =========================================================================
#pragma pack(push, 1) 
struct PacketHeader {   // 모든 패킷의 맨 앞에 위치하여 데이터의 이정표 역할을 함
    uint16_t signature = 0x4543; // 기본값 할당 ('E','C' - Eats Connect)
    CmdID cmdId;                 // 명령어 ID (어떤 파트의 어떤 요청인지 식별)
    uint32_t bodySize;           // 뒤따라오는 JSON 바디의 실제 크기 (바이트 단위)
};
#pragma pack(pop) // [주의] DTO 선언 전에 반드시 pack 설정을 해제해야 함!

// =========================================================================
// 3. 데이터 전송 객체 (DTO) 정의
// 각 역할(Role)에 상관없이 JSON 직렬화를 통해 유연하게 데이터를 주고받음
// =========================================================================

// 회원가입 요청 DTO (통합형)
struct SignupDTO {     
    // 공통 필드
    std::string userId;    // 로그인 ID
    std::string password;  // 비밀번호
    std::string userName;  // 사용자 이름
    std::string phoneNumber; // 전화번호
    int role;              // 0: 고객, 1: 사장님, 2: 라이더

    // 선택적 필드 (역할에 따라 클라이언트에서 선별적으로 채워 보냄)
    std::string address;         // 고객용
    std::string businessNumber;  // 사장님용
    std::string storeName;       // 사장님용
    std::string transportType;   // 라이더용

    // JSON 자동 매핑 (변수명이 JSON의 Key가 됨)
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SignupDTO,  
        userId, password, userName, phoneNumber, role,
        address, businessNumber, storeName, transportType)
};

// 로그인 요청 DTO
struct LoginDTO {  
    std::string userId;  
    std::string password; 
    int role;              // 0: 고객, 1: 사장님, 2: 라이더

    // JSON 자동 매핑
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginDTO, userId, password, role)  
};