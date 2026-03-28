#pragma once
#include <QString>

// ============================================================
// UserSession - 로그인한 유저 정보를 앱 전역에서 공유하는 싱글톤
//
// 사용법:
//   UserSession::instance().userName  → 유저 이름 읽기
//   UserSession::instance().set(...)  → 로그인/회원가입 성공 시 저장
//   UserSession::instance().clear()   → 로그아웃 시 초기화
// ============================================================
class UserSession
{
public:
    // 싱글톤 인스턴스 반환
    static UserSession& instance()
    {
        static UserSession s;
        return s;
    }

    // ── 유저 정보 ──
    QString userName;    // 이름 (홈 화면 등에서 표시)
    QString address;     // 주소 (홈 화면 헤더에 표시)
    QString userId;      // 아이디 (필요 시 사용)
    QString phoneNumber; // 전화번호 (마이페이지에서 표시)
    QString customerGrade;

    // 로그인/회원가입 성공 시 한 번에 저장
    void set(const QString &name, const QString &addr,
             const QString &id = "", const QString &phone = "")
    {
        userName    = name;
        address     = addr;
        userId      = id;
        phoneNumber = phone;
    }

    // 로그아웃 시 전체 초기화
    void clear()
    {
        userName.clear();
        address.clear();
        userId.clear();
        phoneNumber.clear();
    }

    // 로그인 상태 여부
    bool isLoggedIn() const { return !userName.isEmpty(); }

private:
    // 외부에서 생성 불가 (싱글톤)
    UserSession() = default;
    UserSession(const UserSession&) = delete;
    UserSession& operator=(const UserSession&) = delete;
};
