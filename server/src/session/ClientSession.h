// 연결된 클라이언트 한 명의 상태 정보를 담는 상자입니다.
#pragma once
#include <string>

struct ClientSession  // 클라이언트 세션 구조체
{
    int fd;   // 클라이언트 소켓 파일 디스크립터
    std::string recvBuffer;  // 클라이언트로부터 받은 데이터를 저장하는 버퍼
};