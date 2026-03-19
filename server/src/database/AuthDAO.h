#pragma once
#include <string>
#include <nlohmann/json.hpp>

class AuthDAO
{
public:
    static AuthDAO &getInstance()
    {
        static AuthDAO instance;
        return instance;
    }

    // 로그인 검증 전용 함수
    // 성공 시 유저 기본 정보 반환, 실패 시 빈 객체 반환
    nlohmann::json validateLogin(const std::string &userId, const std::string &password)
    {
        // 1. DB에서 해당 ID의 해시된 비밀번호와 role을 가져옴
        // 2. 입력받은 password와 DB의 해시값을 비교
        // 3. 일치하면 유저 정보를 반환
        return nlohmann::json::object();
    }

private:
    AuthDAO() = default;
};