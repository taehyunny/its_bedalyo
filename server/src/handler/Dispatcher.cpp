#include "Dispatcher.h"
#include "UserHandler.h"
#include "OrderHandler.h"
#include "SystemHandler.h"
#include <iostream>

void Dispatcher::dispatch(ClientSession* session, const PacketHeader& header, const std::string& jsonBody) {
    std::cout << "[Dispatcher] 수신된 CmdID: " << static_cast<int>(header.cmdId) << std::endl;

    switch (header.cmdId) {
        case CmdID::REQ_SIGNUP:
            // 직접 호출은 삭제하고, 오직 스레드풀에만 던집니다.
            g_threadPool.enqueue(&UserHandler::handleSignup, session, jsonBody);
            break;

        case CmdID::REQ_LOGIN:
            // Signup이 아닌 Login 핸들러를 정확히 매핑해야 합니다.
            g_threadPool.enqueue(&UserHandler::handleLogin, session, jsonBody);
            break;

        case CmdID::REQ_HEARTBEAT:
            g_threadPool.enqueue(&SystemHandler::handleHeartbeat, session, jsonBody);
            break;

        case CmdID::REQ_STORE_LIST:
        case CmdID::REQ_ORDER_CREATE:
            g_threadPool.enqueue(&OrderHandler::handleOrder, session, header.cmdId, jsonBody);
            break;

        default:
            std::cerr << "[WARNING] 알 수 없는 명령어: " << static_cast<int>(header.cmdId) << std::endl;
            break;
    }
}