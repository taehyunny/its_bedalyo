// TCP 스트림에서 8바이트 헤더와 JSON 바디를 잘라내는 역할

#pragma once
#include <vector>
#include <string>
#include "../session/ClientSession.h"
#include "../include/Global_protocol.h"

class PacketFramer {
public:
    // ItsServer에서 호출할 메인 진입점
    static void onReceiveData(ClientSession* session, const char* data, int len);

private:
    // 실제 조립 로직 (내부에서 루프를 돌며 처리)
    static void processBuffer(ClientSession* session);
};