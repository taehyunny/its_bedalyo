#pragma once
#include <QString>

// ============================================================
// AppConfig - 앱 전역 설정 상수
// ============================================================
namespace AppConfig
{
    // ── 서버 연결 ──
    static const QString SERVER_IP   = "10.10.10.123";
    static const quint16 SERVER_PORT = 8004;


    // ── 앱 기본 설정 ──
    static const int WINDOW_WIDTH  = 390;
    static const int WINDOW_HEIGHT = 844;

    // ── 도로명주소 API (juso.go.kr) ──
    static const QString JUSO_API_KEY = "devU01TX0FVVEgyMDI2MDMyMzIwMjkyNTExNzc3NjI=";
    static const QString JUSO_API_URL = "https://business.juso.go.kr/addrlink/addrLinkApi.do";
}

