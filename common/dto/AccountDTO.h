#pragma once
#include <string>
//#include <nlohmann/json.hpp>
#include "json.hpp"

// ---------------------------------------------------------
// [1] 회원가입 요청 DTO (클라이언트 -> 서버)
// ---------------------------------------------------------
struct SignupReqDTO {
    std::string userId;      // 아이디
    std::string password;    // 비밀번호 (서버에서 해싱 처리 예정)
    std::string userName;    // 유저 이름 (핸들러의 req.userName에 대응)
    std::string phoneNumber; // 전화번호
    int role;                // 0: 고객, 1: 사장님, 2: 관리자/라이더

    // JSON <-> Struct 자동 변환 매크로
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SignupReqDTO, userId, password, userName, phoneNumber, role)
};

// ---------------------------------------------------------
// [2] 로그인 요청 DTO (클라이언트 -> 서버)
// ---------------------------------------------------------
struct LoginReqDTO {
    std::string userId;      // 아이디 (핸들러의 req.userId에 대응)
    std::string password;    // 비밀번호 (핸들러의 req.password에 대응)

    // JSON <-> Struct 자동 변환 매크로
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginReqDTO, userId, password)
};

// ---------------------------------------------------------
// [3] 통합 인증 응답 DTO (서버 -> 클라이언트)
// ---------------------------------------------------------
// 회원가입과 로그인 모두 상태값(status), 메시지(message), 유저이름(userName)을 반환하므로 하나로 통합합니다.
struct AuthResDTO {
    int status;          // 200: 성공, 401: 실패, 409: 중복
    std::string message; // "로그인 성공" 또는 "이미 가입된 아이디입니다."
    std::string userName;// 클라이언트 UI 상단 "OOO님 환영합니다" 표시용

    // JSON <-> Struct 자동 변환 매크로
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AuthResDTO, status, message, userName)
};