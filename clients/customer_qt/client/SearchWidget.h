#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SearchWidget; }
QT_END_NAMESPACE

// ============================================================
// 최근 검색어 항목 (서버 응답 DTO 확정 전 임시 구조체)
// 서버팀과 테이블 확정 후 SearchDTO.h 로 분리 예정
// ============================================================
struct RecentSearchItem {
    int     searchId;   // 검색어 PK (개별 삭제 시 서버에 전송)
    QString keyword;    // 검색어
    QString date;       // 날짜 문자열 (예: "03.21")
};

// ============================================================
// SearchWidget - 검색 화면
//
// [ 기능 ]
// 1. 화면 진입 시 REQ_RESEACH_WIDGET(2108) + userId 전송
//    → 서버에서 인기검색어(1~5위) + 최근검색어 리스트 수신
// 2. 검색 실행 (btnSearch 클릭 or Enter)
//    → 입력값 없으면 경고, 있으면 검색 결과 화면 전환 + REQ_RESEARCH_ADD(2112) 전송
// 3. 인기검색어 / 최근검색어 클릭
//    → searchEdit 자동 입력 후 바로 검색 실행
// 4. 최근검색어 개별 삭제 (X 버튼)
//    → UI 즉시 제거 + REQ_RESEARCH_DELETE(2110) 전송
// 5. 최근검색어 전체 삭제 (전체삭제 버튼)
//    → UI 전체 제거 + recentSection 숨김 + REQ_RESEARCH_DEL_ALL(2114) 전송
// 6. ← 버튼 → backRequested 시그널 emit (MainWindow에서 홈으로 전환)
//
// [ 서버 연동 프로토콜 ]
// REQ_RESEACH_WIDGET  = 2108 / RES_RESEACH_WIDGET  = 2109
// REQ_RESEARCH_DELETE = 2110 / RES_RESEARCH_DELETE = 2111
// REQ_RESEARCH_ADD    = 2112 / RES_RESEARCH_ADD    = 2113
// REQ_RESEARCH_DEL_ALL= 2114 / RES_RESEARCH_DEL_ALL= 2115
// ============================================================
class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~SearchWidget();

    // MainWindow에서 화면 전환 직후 호출 → 서버에 위젯 데이터 요청
    void loadSearchData();

signals:
    void backRequested();                        // ← 버튼 → 홈으로
    void searchRequested(const QString &keyword); // 검색 실행 → 결과 화면으로
    void favoriteRequested();
    void orderListRequested();
    void mypageRequested();

private slots:
    // ── 사용자 액션 ──
    void on_btnBack_clicked();
    void on_btnSearch_clicked();
    void on_btnDeleteAll_clicked();
    void onSearchEditReturnPressed();

    // ── 서버 응답 슬롯 (NetworkManager 시그널 연결) ──
    // TODO: NetworkManager에 아래 시그널 추가 필요
    // void onSearchWidgetReceived(QList<QString> popular, QList<RecentSearchItem> recent);
    // void onRecentDeleteReceived(int status);
    // void onRecentAddReceived(int status);
    // void onRecentDeleteAllReceived(int status);

    // ── 내비바 ──
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

private:
    Ui::SearchWidget *ui;
    NetworkManager   *m_network;

    QList<RecentSearchItem> m_recentItems; // 현재 표시 중인 최근 검색어 목록

    // ── UI 빌더 ──
    void buildPopularList(const QList<QString> &keywords);
    void buildRecentList(const QList<RecentSearchItem> &items);
    void clearLayout(QLayout *layout);

    // ── 검색 실행 내부 로직 ──
    void executeSearch(const QString &keyword);

    // ── 최근 검색어 단건 삭제 (UI + 서버) ──
    void deleteRecentItem(int searchId, const QString &keyword);

    // ── 위젯 생성 헬퍼 ──
    QWidget* makeRecentItemWidget(const RecentSearchItem &item);
};
