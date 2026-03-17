// 로그인/회원가입 요청 처리

#include "UserHandler.h"
#include "Global_protocol.h"
#include "ItsServer.h"
#include <iostream>

void UserHandler::handleSignup(int clientFd, const std::string &jsonBody, ItsServer *server)
{
    json responseJson; // 클라이언트에게 돌려줄 빈 응답 상자

    try
    {
        // 1. 역직렬화: 수신된 JSON 텍스트를 파싱
        json requestJson = json::parse(jsonBody);

        // 2. 마법의 1:1 매핑: JSON 데이터를 통째로 SignupDTO 구조체에 집어넣음
        SignupDTO signupData = requestJson.get<SignupDTO>();

        std::cout << "[회원가입 처리] ID: " << signupData.userId
                  << ", 이름: " << signupData.userName
                  << ", 역할: " << signupData.role << std::endl;

        // ---------------------------------------------------------
        // 3. DB 저장 로직 (나중에 UserDAO::insertUser(signupData)로 대체될 부분)
        // ---------------------------------------------------------
        bool isDbInsertSuccess = true; // 지금은 무조건 성공한다고 가정합니다.

        // 4. 결과에 따라 응답 상자(JSON)에 내용물 채우기
        if (isDbInsertSuccess)
        {
            responseJson["status"] = 200;
            responseJson["message"] = signupData.userName + "님, 이츠 배달료 가입을 환영합니다!";
        }
        else
        {
            responseJson["status"] = 400;
            responseJson["message"] = "이미 사용 중인 아이디입니다.";
        }
    }
    catch (const json::exception &e)
    {
        // 클라이언트가 필수 값을 빼먹었거나 형식이 틀렸을 때의 방어 로직
        responseJson["status"] = 500;
        responseJson["message"] = "데이터 형식이 올바르지 않습니다.";
        std::cerr << "[에러] JSON 파싱 실패: " << e.what() << std::endl;
    }

    // 5. 발송 부서(ItsServer) 호출!
    // "1021번(RES_SIGNUP) 송장 붙여서 이 손님(clientFd)한테 바로 쏴주세요!"
    server->sendPacket(clientFd, CmdID::RES_SIGNUP, responseJson);
}