// USERS 테이블 접근
#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "common/dto/AllDTOs.h"

class UserDAO
{
public:
    // 싱글톤 인스턴스 반환
    static UserDAO &getInstance()
    {
        static UserDAO instance;
        return instance;
    }

    // 회원가입 (성공 시 true)
    bool insertUser(const SignupReqDTO &req)
    {
        // TODO: 실제 MariaDB INSERT 쿼리 구현
        return true;
    }

    // 로그인 (유저 정보 JSON 반환)
    nlohmann::json selectUser(const std::string &userId, const std::string &password)
    {
        // TODO: 실제 MariaDB SELECT 쿼리 구현
        nlohmann::json mockUser;
        // 테스트용 데이터
        if (userId == "test")
        {
            mockUser["user_name"] = "태현";
            mockUser["role"] = 0;
        }
        return mockUser;
    }

private:
    UserDAO() {} // 외부 생성 방지
};