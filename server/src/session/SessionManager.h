// /서버에 접속한 수천 명의 ClientSession을 총괄 관리하는 지휘통제실입니다.
#pragma once
#include <string>
#include <unordered_map>
#include "ClientSession.h"

class SessionManager  // 세션 매니저 클래스
{
    std::unordered_map<int, ClientSession *> sessionMap; // 클라이언트 소켓 파일 디스크립터를 키로, ClientSession 포인터를 값으로 하는 해시맵

    ClientSession *getSession(int fd) // 클라이언트 소켓 파일 디스크립터로 세션을 조회하는 함수
    {
        return sessionMap[fd]; // 1만 명 중 특정 손님의 방을 0.001초 만에 찾아줌
    }
};