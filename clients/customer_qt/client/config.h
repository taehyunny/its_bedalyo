#pragma once
#include <QString>

// ============================================================
// AppConfig - 앱 전역 설정 상수
//
// 서버 IP/Port 등 변경 가능성이 있는 값들을 한 곳에서 관리
// 서버 주소가 바뀌면 이 파일만 수정하면 됩니다
// ============================================================
namespace AppConfig
{
    // ── 서버 연결 ──
    static const QString SERVER_IP   = "10.10.10.123";
    static const quint16 SERVER_PORT = 8003;

    // ── 앱 기본 설정 ──
    static const int WINDOW_WIDTH  = 390;
    static const int WINDOW_HEIGHT = 844;
}
