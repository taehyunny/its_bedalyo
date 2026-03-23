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
// 최근 검색어 항목 — ResearchDTO.h의 RecentSearch Qt 래핑
// historyId : 서버 PK (개별 삭제 시 전송)
// keyword   : 검색어
// (date 필드 제거 — 서버 DTO에 없음)
// ============================================================
struct RecentSearchItem {
    int     historyId;  // 서버 PK (개별 삭제 시 전송)
    QString keyword;    // 검색어
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
    void backRequested();
    void searchRequested(const QString &keyword);
    void favoriteRequested();
    void orderListRequested();
    void mypageRequested();

private slots:
    // ── 사용자 액션 ──
    void on_btnBack_clicked();
    void on_btnSearch_clicked();
    void on_btnDeleteAll_clicked();
    void onSearchEditReturnPressed();

    // ── 서버 응답 슬롯 ──
    void onSearchWidgetReceived(QList<PopularKeywordQt> popular,
                                QList<RecentSearchQt> recent);

    // ── 내비바 ──
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

private:
    Ui::SearchWidget *ui;
    NetworkManager   *m_network;

    QList<RecentSearchItem> m_recentItems;

    // ── UI 빌더 ──
    void buildPopularList(const QList<PopularKeywordQt> &keywords);
    void buildRecentList(const QList<RecentSearchItem> &items);
    void clearLayout(QLayout *layout);

    // ── 검색 실행 내부 로직 ──
    void executeSearch(const QString &keyword);

    // ── 최근 검색어 단건 삭제 (UI + 서버) ──
    void deleteRecentItem(int historyId, const QString &keyword);

    // ── 위젯 생성 헬퍼 ──
    QWidget* makeRecentItemWidget(const RecentSearchItem &item);
};
