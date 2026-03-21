#pragma once

#include <QWidget>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyPageWidget; }
QT_END_NAMESPACE

// ============================================================
// MyPageWidget - 마이이츠 화면
//
// [ 구성 ]
// - 프로필: 이름, 전화번호, 리뷰수/도움됐어요수/즐겨찾기수, 자세히보기 버튼
// - 메뉴: 주소관리, 즐겨찾기, 쿠폰·이용권, 자주묻는질문, 약관및정책, 설정, 로그아웃
// - 전체 스크롤
//
// [ 서버 연동 ]
// REQ_MY_INFO(2100) / RES_MY_INFO(2101) — 리뷰수/도움됐어요/즐겨찾기수
// TODO: 서버팀 DTO 확정 후 구현
//
// [ TODO 목록 ]
// - 자세히 보기 → 프로필 상세 화면 (미구현)
// - 주소 관리   → 주소 관리 화면 (미구현)
// - 즐겨찾기    → 즐겨찾기 화면 (미구현)
// - 쿠폰·이용권 → 쿠폰 화면 (미구현)
// - 자주 묻는 질문 → FAQ 화면 (미구현)
// - 약관 및 정책  → 정책 화면 (미구현)
// - 설정          → 설정 화면 (미구현)
// ============================================================
class MyPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyPageWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~MyPageWidget();

    // MainWindow에서 화면 진입 시 호출
    void loadData();

    // 통계 카운트 업데이트 (서버 응답 수신 후 호출)
    void setStats(int reviewCount, int helpCount, int favoriteCount);

signals:
    void homeRequested();
    void searchRequested();
    void favoriteRequested();
    void orderListRequested();
    void logoutRequested();

    // TODO: 각 메뉴 화면 전환 시그널
    // void addressRequested();
    // void couponRequested();
    // void faqRequested();
    // void policyRequested();
    // void settingsRequested();
    // void detailRequested();

private slots:
    void on_btnDetail_clicked();
    void on_btnAddress_clicked();
    void on_btnFavorite_clicked();
    void on_btnCoupon_clicked();
    void on_btnFaq_clicked();
    void on_btnPolicy_clicked();
    void on_btnSettings_clicked();
    void on_btnLogout_clicked();

    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

    // TODO: 서버 응답 슬롯
    // void onMyInfoReceived(int reviewCount, int helpCount, int favoriteCount);

private:
    Ui::MyPageWidget *ui;
    NetworkManager   *m_network;

    // UserSession 정보로 프로필 UI 채우기
    void updateProfileUI();

    // 전화번호 마스킹 (예: 010-1234-5678 → 010-****-5678)
    static QString maskPhoneNumber(const QString &phone);
};
