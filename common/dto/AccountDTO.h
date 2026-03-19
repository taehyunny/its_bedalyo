#pragma once
#include <string>
<<<<<<< Updated upstream
// #include <nlohmann/json.hpp>
#include "json.hpp" // nlohmann json
// ---------------------------------------------------------
// [1] 회원가입 요청 DTO (클라이언트 -> 서버)
// ---------------------------------------------------------
struct SignupReqDTO
{
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
struct LoginReqDTO
{
    std::string userId;   // 아이디 (핸들러의 req.userId에 대응)
    std::string password; // 비밀번호 (핸들러의 req.password에 대응)

    // JSON <-> Struct 자동 변환 매크로
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginReqDTO, userId, password)
};

// ---------------------------------------------------------
// [3] 통합 인증 응답 DTO (서버 -> 클라이언트)
// ---------------------------------------------------------
// 회원가입과 로그인 모두 상태값(status), 메시지(message), 유저이름(userName)을 반환하므로 하나로 통합합니다.
struct AuthResDTO
{
    int status;
    std::string message;
    std::string userName;

    // 🚀 이 매크로가 반드시 구조체 정의 바로 아래(혹은 안)에 있어야 합니다!
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthResDTO, status, message, userName)
=======
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

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResDTO, status, message, userName)
>>>>>>> Stashed changes
};