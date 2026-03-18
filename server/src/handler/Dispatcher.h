#pragma once

#include "../include/Global_protocol.h"
#include "../session/ClientSession.h"
#include <string>

class Dispatcher {
public:
    // PacketFramer에서 완성된 패킷을 넘겨받는 메인 진입점
    static void dispatch(ClientSession* session, const PacketHeader& header, const std::string& jsonBody);

private:
    // 인스턴스 생성을 막아 유틸리티 클래스임을 명시 (Jensen Huang 스타일의 효율성)
    Dispatcher() = delete;
};