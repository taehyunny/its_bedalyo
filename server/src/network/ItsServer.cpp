#include "ItsServer.h"
#include "../session/SessionManager.h"
#include "PacketFramer.h" // PacketFramer 헤더 포함 필수

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // sockaddr_in 등 네트워크 구조체용
#include <fcntl.h>     // fcntl (논블로킹 설정용)
#include <unistd.h>    // close, read 함수용
#include <cstring>     // memset
#include <cerrno>      // errno, EAGAIN, EWOULDBLOCK 사용을 위해 필수!

// ==========================================================
// 1. 서버 초기화 (생성자)
// ==========================================================
ItsServer::ItsServer(int port)
{
    sessionManager = new SessionManager();
    events = new epoll_event[MAX_EVENTS];

    // 1. 대표 전화 개통 (TCP 소켓)
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1)
    {
        std::cerr << "[FATAL] 서버 소켓 생성 실패" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. 주소 할당 및 바인딩
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // 포트 재사용 옵션 (서버 재시작 시 포트 충돌 방지)
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "[FATAL] 포트 " << port << " 바인딩 실패 (이미 사용 중일 수 있음)" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, SOMAXCONN) == -1)
    {                                                    // 최대 대기열 크기로 listen 시작
        std::cerr << "[FATAL] listen 실패" << std::endl; // 치명적인 오류로 간주하여 프로그램 종료
        exit(EXIT_FAILURE);                              // 치명적인 오류로 간주하여 프로그램 종료
    }

    // 서버 소켓을 논블로킹 모드로 전환
    setNonBlocking(serverFd);

    // epoll 문지기 고용
    epollFd = epoll_create1(0); // 0은 기본 옵션, 실패 시 -1 반환
    if (epollFd == -1)
    {                                                           // epoll 생성 실패 시 치명적인 오류로 간주
        std::cerr << "[FATAL] epoll_create1 실패" << std::endl; // 치명적인 오류로 간주하여 프로그램 종료
        exit(EXIT_FAILURE);
    }

    // 서버 소켓을 epoll 감시망에 등록 (엣지 트리거 방식)
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = serverFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &event) == -1)
    {
        std::cerr << "[FATAL] epoll_ctl (서버 소켓 등록) 실패" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "[ItsServer] 포트 " << port << "에서 이츠 배달료 서버 정상 가동!" << std::endl;
}

// ==========================================================
// 2. 서버 자원 해제 (소멸자)
// ==========================================================
ItsServer::~ItsServer()
{
    close(serverFd);       // 대표 전화번호 소켓 닫기
    close(epollFd);        // 문지기 소켓 닫기
    delete[] events;       // 이벤트 바구니 해제
    delete sessionManager; // 프론트 데스크 해제
    std::cout << "[ItsServer] 서버가 안전하게 종료되었습니다." << std::endl;
}

// ==========================================================
// 3. 논블로킹 설정 유틸리티
// ==========================================================
void ItsServer::setNonBlocking(int sockFd)
{
    int flags = fcntl(sockFd, F_GETFL, 0);
    if (flags == -1)
        return;
    fcntl(sockFd, F_SETFL, flags | O_NONBLOCK);
}

// ==========================================================
// 4. 메인 이벤트 루프 (생명 주기)
// ==========================================================
void ItsServer::run()
{
    while (true)
    {
        int event_count = epoll_wait(epollFd, events, MAX_EVENTS, -1);

        if (event_count == -1)
        {
            if (errno == EINTR)
                continue; // 시스템 인터럽트 발생 시 무시하고 계속 진행
            std::cerr << "[ERROR] epoll_wait 오류 발생" << std::endl;
            break;
        }

        for (int i = 0; i < event_count; ++i)
        {
            int currentFd = events[i].data.fd;

            if (currentFd == serverFd)
            {
                // 새로운 클라이언트 접속 시도
                acceptNewClient();
            }
            else
            {
                // 기존 클라이언트가 보낸 데이터 수신
                readFromClient(currentFd);
            }
        }
    }
}

// ==========================================================
// 5. 신규 클라이언트 수락 (Accept)
// ==========================================================
void ItsServer::acceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // 엣지 트리거이므로, 대기열에 있는 모든 손님을 한 번에 다 받음
    while (true)
    {
        int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientFd == -1)
        {
            // 더 이상 수락할 손님이 없으면 (EAGAIN) 탈출
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                std::cerr << "[ERROR] accept 실패" << std::endl;
                break;
            }
        }

        // 새 소켓 역시 논블로킹 설정
        setNonBlocking(clientFd);

        // 프론트 데스크에 새 객실 등록 (Session 생성)
        sessionManager->createSession(clientFd);

        // 새 클라이언트 소켓을 epoll 감시망에 등록
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = clientFd;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &event);

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
        std::cout << "[INFO] 새 클라이언트 접속 완료! (fd: " << clientFd << ", IP: " << ipStr << ")" << std::endl;
    }
}

// ==========================================================
// 6. 데이터 수신 및 분리 (Read & Frame)
// ==========================================================
void ItsServer::readFromClient(int clientFd)
{
    char buffer[4096];

    // 논블로킹 소켓이므로, 버퍼에 있는 데이터를 남김없이 다 긁어와야 함
    while (true)
    {
        int readBytes = read(clientFd, buffer, sizeof(buffer));

        if (readBytes > 0)
        {
            // [정상 수신] 프론트 데스크에서 이 손님의 객실(Session)을 찾아옴
            // 주의: SessionManager의 구현체에 getSession 메서드가 있어야 합니다.
            ClientSession *session = sessionManager->getSession(clientFd);

            if (session != nullptr)
            {
                // PacketFramer에게 "이 파편들 조립해서 상자 만들어줘!" 라고 넘김
                PacketFramer::onReceiveData(session, buffer, readBytes);
            }
        }
        else if (readBytes == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 소켓 버퍼를 다 비웠음
                break;
            }
            else
            {
                std::cerr << "[ERROR] read 실패" << std::endl;
                disconnectClient(clientFd);
                break;
            }
        }
        else
        {
            // 클라이언트가 연결을 종료했음
            disconnectClient(clientFd);
            break;
        }
    }
}