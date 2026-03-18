#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <unistd.h> // write 함수용
#include <nlohmann/json.hpp>
#include "Global_protocol.h"

class ClientSession
{
public:
    explicit ClientSession(int fd) : clientFd(fd), isAuth(false), role(0) {}

    // [역할 1] 원본 데이터를 버퍼에 적재 (ItsServer가 호출)
    void appendToReadBuffer(const char *data, size_t len)
    {
        readBuffer.insert(readBuffer.end(), data, data + len);
    }

    // [역할 2] 패킷 분석을 위해 버퍼 참조 제공 (PacketFramer가 사용)
    const std::vector<char> &getReadBuffer() const { return readBuffer; }

    // [역할 3] 분석이 끝난 데이터를 버퍼에서 제거 (PacketFramer가 요청)
    void removeReadData(size_t len)
    {
        if (len <= readBuffer.size())
        {
            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + len);
        }
    }

    // [역할 4] 유저의 신원 보존 (Handler가 사용)
    void authenticate(const std::string &id, int userRole)
    {
        userId = id;
        role = userRole;
        isAuth = true;
    }
    template <typename T>
    void sendPacket(CmdID cmdId, const T &dto)
    {
        try
        {
            // 1. DTO -> JSON 문자열 직렬화 (nlohmann::json의 마법)
            nlohmann::json j = dto;
            std::string jsonBody = j.dump();

            // 2. 패킷 헤더 조립 (Endian 변환이 필요하다면 여기서 htons/htonl 적용)
            PacketHeader header;
            header.cmdId = cmdId;
            header.bodySize = static_cast<uint32_t>(jsonBody.length());
            // header.signature = 0xAA55; // (팀에서 약속한 시그니처가 있다면 추가)

            // 3. 전송용 버퍼에 헤더와 바디 결합
            std::vector<char> sendBuffer;
            sendBuffer.insert(sendBuffer.end(), reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(PacketHeader));
            sendBuffer.insert(sendBuffer.end(), jsonBody.begin(), jsonBody.end());

            // 4. 소켓 전송 (워커 스레드 경합 방지용 자물쇠)
            std::lock_guard<std::mutex> lock(writeMutex);
            ssize_t sent = write(clientFd, sendBuffer.data(), sendBuffer.size());

            if (sent < 0)
            {
                // 전송 실패 로그 처리 (클라이언트가 강제 종료했을 수 있음)
            }
        }
        catch (const std::exception &e)
        {
            // 직렬화 실패 예외 처리
        }
    }

private:
    int clientFd;
    std::vector<char> readBuffer;

    bool isAuth;
    std::string userId;
    int role;

    std::mutex sessionMutex; // 상태 보호용
    std::mutex writeMutex;   // 🚀 소켓 전송(write) 동시 접근 보호용
};