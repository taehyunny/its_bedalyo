// epoll 루프, 클라이언트 세션 관리, Heartbeat 유지

#pragma once

#include <sys/epoll.h>
#include <vector>

// 전방 선언 (헤더 꼬임 방지)
class SessionManager;

class ItsServer {
private:
    int serverFd;      // 식당의 대표 전화번호 (접속 대기용 소켓)
    int epollFd;       // 문지기 (이벤트 감시기)
    struct epoll_event* events; // 발생한 이벤트들을 담을 바구니
    const int MAX_EVENTS = 10000; // 한 번에 처리할 최대 이벤트 수

    SessionManager* sessionManager; // 프론트 데스크 (세션 관리자)

    // 소켓을 논블로킹 모드로 바꾸는 마법의 유틸리티 함수
    void setNonBlocking(int sockFd);

public: // 생성자와 소멸자
    ~ItsServer();  // 소멸자: 자원 해제

    void run(); // 서버 무한 루프 시작

private:
    void acceptNewClient();   // 새로운 손님이 접속했을 때 처리하는 함수
    void readFromClient(int clientFd);  // 이미 접속한 손님이 데이터를 보냈을 때 처리하는 함수
    void disconnectClient(int clientFd);  // 손님이 나갔을 때 처리하는 함수
};