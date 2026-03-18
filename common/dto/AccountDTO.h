#pragma once
#include <string>
#include "json.hpp"

struct SignupReqDTO {  // 회원가입 요청 DTO
    std::string userId;   // 회원 ID (로그인 시 사용)
    std::string password;  // 비밀번호
    std::string userName;  // 사용자 이름 (환영 메시지 등에 사용)
    std::string phoneNumber;  // 전화번호 (인증, 연락용)
    int role; // 0: 고객, 1: 사장님, 2: 라이더

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SignupReqDTO, userId, password, userName, phoneNumber, role)
};

struct LoginReqDTO {  // 로그인 요청 DTO
    std::string userId;   // 회원 ID (SignupReqDTO의 userId와 동일한 값)
    std::string password; // 비밀번호
    int role;               // 0: 고객, 1: 사장님, 2: 라이더 (클라이언트가 어떤 유형의 로그인 시도인지 명확히 서버에 전달)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginReqDTO, userId, password, role)
};

struct LoginResDTO {   // 로그인 응답 DTO
    int status;          // 0: 성공, 1: 실패 (예: 아이디/비밀번호 불일치, 계정 정지 등)
    std::string message; // 로그인 결과 메시지 (성공 시 "로그인 성공", 실패 시 구체적인 이유)
    std::string userName; // 환영 메시지용

    // 가게가 5개가 아니라 여러 개가 될 수도 있다는 제약 사항을 고려해 vector로 처리
    std::vector<int> ownerStoreIds; // [추가된 부분] 사장님이 로그인했을 때 자신이 관리하는 상점 ID 목록을 받기 위함

    // [수정] ownerStoreIds 추가
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResDTO, status, message, userName, ownerStoreIds)
};