#pragma once

#include <sys/epoll.h>
#include <vector>
#include <string>
#include "Global_protocol.h" // CmdID와 PacketHeader 사용
#include "json.hpp"                    // nlohmann json

using json = nlohmann::json;

class SessionManager; // 전방 선언

class ItsServer {
private:
    int serverFd;
    int epollFd;
    struct epoll_event* events;
    const int MAX_EVENTS = 10000;

    SessionManager* sessionManager;

    void setNonBlocking(int sockFd);

public:
    ItsServer(int port);
    ~ItsServer();

    void run(); // 메인 이벤트 루프

    // 비즈니스 로직(Handler)에서 ItsServer를 통해 클라이언트에게 응답을 보낼 때 사용하는 유일한 창구
    void sendPacket(int clientFd, CmdID cmd, const json& responseData);

private:
    void acceptNewClient();
    void readFromClient(int clientFd);
    void disconnectClient(int clientFd);
};