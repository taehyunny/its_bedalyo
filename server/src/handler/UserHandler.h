#pragma once
#include <string>

// 중요: ItsServer가 클래스라는 걸 미리 알려줌 (전방 선언)
class ItsServer; 

class UserHandler {
public:
    static void handleSignup(int clientFd, const std::string& jsonBody, ItsServer* server);
};