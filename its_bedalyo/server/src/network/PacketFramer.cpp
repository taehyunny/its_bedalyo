#include "PacketFramer.h"
#include "../include/Global_protocol.h"
#include "../handler/Dispatcher.h"
#include <cstring>

void PacketFramer::onReceiveData(ClientSession* session, const char* data, int len) {
    // 1. 일단 세션의 보관함에 데이터를 집어넣습니다.
    session->appendToReadBuffer(data, len);

    // 2. 보관함에 패킷이 완성되었는지 검사합니다.
    processBuffer(session);
}

void PacketFramer::processBuffer(ClientSession* session) {
    // 세션의 버퍼를 가져옵니다.
    const auto& buffer = session->getReadBuffer();

    // 헤더(8바이트)가 올 때까지 루프
    while (buffer.size() >= sizeof(PacketHeader)) {
        PacketHeader header;
        std::memcpy(&header, buffer.data(), sizeof(PacketHeader));

        // [검사 1] 우리 서비스 패킷이 맞는가? (Signature 체크)
        if (header.signature != 0x4543) {
            // 비정상적인 접근은 연결 종료
            // ItsServer::disconnect(session->getFd()); 
            return;
        }

        // [검사 2] 바디(JSON)까지 다 도착했는가?
        uint32_t totalPacketSize = sizeof(PacketHeader) + header.bodySize;
        if (buffer.size() < totalPacketSize) {
            break; // 아직 덜 왔으니 다음 read를 기다림
        }

        // [실행 1] 온전한 JSON 바디만 추출
        std::string jsonBody(buffer.begin() + sizeof(PacketHeader), 
                             buffer.begin() + totalPacketSize);

        // [실행 2] 완성된 패킷을 디스패처(우체국장)에게 전달
        Dispatcher::dispatch(session, header, jsonBody);

        // [실행 3] 세션에게 사용한 만큼 지우라고 명령 (가장 중요!)
        session->removeReadData(totalPacketSize);
    }
}