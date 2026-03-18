#pragma once
#include <vector>
#include <string>

class ClientSession {
public:
    explicit ClientSession(int fd) : clientFd(fd), isAuth(false), role(0) {}

    // [역할 1] 원본 데이터를 버퍼에 적재 (ItsServer가 호출)
    void appendToReadBuffer(const char* data, size_t len) {
        readBuffer.insert(readBuffer.end(), data, data + len);
    }

    // [역할 2] 패킷 분석을 위해 버퍼 참조 제공 (PacketFramer가 사용)
    const std::vector<char>& getReadBuffer() const { return readBuffer; }

    // [역할 3] 분석이 끝난 데이터를 버퍼에서 제거 (PacketFramer가 요청)
    void removeReadData(size_t len) {
        if (len <= readBuffer.size()) {
            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + len);
        }
    }

    // [역할 4] 유저의 신원 보존 (Handler가 사용)
    void authenticate(const std::string& id, int userRole) {
        userId = id;
        role = userRole;
        isAuth = true;
    }

    int getFd() const { return clientFd; }
    bool isAuthenticated() const { return isAuth; }

private:
    int clientFd;
    std::vector<char> readBuffer; // 이 연결만의 전용 보관함
    
    // 세션 상태 정보
    bool isAuth;
    std::string userId;
    int role;
};