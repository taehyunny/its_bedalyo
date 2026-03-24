#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StoreDetailWidget; }
QT_END_NAMESPACE

class StoreDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StoreDetailWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~StoreDetailWidget();

    // 가게 ID만 받고, 메뉴 정보는 서버에 직접 요청하도록 변경
    void loadStoreData(int storeId);

signals:
    void backRequested();
    void menuSelected(int menuId, QString menuName, int price);
    // 뒤로가기 버튼 눌림을 알리는 신호
    void backButtonClicked();

private slots:
    void on_btnBack_clicked();
    void onStoreDetailReceived(StoreDetailQt detail);

    void on_btnStoreInfo_clicked();
    void on_btnStoreInfoBack_clicked();
   
    // 메인 화면 뒤로가기 버튼 클릭 슬롯
    void on_btnBackToMain_clicked();
    
    // 별점 버튼 클릭 시 실행될 함수
    void on_btnRating_clicked();
    
private:
    Ui::StoreDetailWidget *ui;
    NetworkManager *m_network;
    int m_currentStoreId; // 현재 보고 있는 가게 ID

    // 마지막으로 받은 가게 상세 정보를 저장해둘 변수
    StoreDetailQt* m_lastStoreData = nullptr;

    // void renderMenuList(const QList<MenuDTO>& menus);
    void clearLayout(QLayout* layout);
    QWidget* makeMenuCard(const MenuQt& menu);

    // 리뷰 관련 함수들
    QWidget* makeReviewCard(const ReviewQt& review);     // 캡쳐화면과 똑같은 리뷰 카드 1개 만들기
    void renderReviews(const QList<ReviewQt>& reviews);  // 리뷰 목록 전체를 화면에 그리기
    void loadDummyReviews();                             // 서버 데이터 오기 전 보여줄 더미 데이터 생성

    // 매장 정보 UI를 그리는 함수들
    void populateStoreInfoPage(); 
    QWidget* createInfoSection(const QString& icon, const QString& title);
    QWidget* createDataRow(const QString& title, const QString& content, bool showMapButton = false);

    void populatePhotoReviewBar(const QList<ReviewQt>& reviews);
    QWidget* createPhotoReviewCard(const ReviewQt& review);
};