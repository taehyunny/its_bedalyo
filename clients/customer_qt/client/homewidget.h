#pragma once
#include <QWidget>
#include <QListWidgetItem>
#include "NetworkManager.h"
#include "StoreItemWidget.h"

// ============================================================
// HomeWidget — 홈 화면
//
// 가게 목록 흐름:
//   1. 탭 진입 or 카테고리 선택
//      → NetworkManager::sendStoreListRequest(category) 호출 (TODO)
//   2. 서버 응답 수신
//      → onStoreListReceived(stores) 슬롯에서 populateStoreList() 호출
//   3. 각 StoreItemWidget::setData() 로 UI 채움
//   4. 이미지 URL 따로 수신 후 setMainImage() 교체
// ============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class HomeWidget; }
QT_END_NAMESPACE

// 서버에서 받아올 가게 데이터 구조체
// 추후 공용 DTO(StoreListResDTO 등)로 교체 예정
struct StoreInfo {
    int         storeId       = 0;
    QString     name;
    QString     category;
    double      rating        = 0.0;
    int         reviewCount   = 0;
    double      distanceKm    = 0.0;
    int         minDeliveryTime = 0;
    int         maxDeliveryTime = 0;
    int         deliveryFee   = 0;
    int         minOrderAmount = 0;
    QStringList tags;
};

class HomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit HomeWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~HomeWidget();

    void setUserName(const QString &userName);

    // 서버 응답 수신 후 외부에서 호출 (NetworkManager 시그널 연결 시 사용)
    void populateStoreList(const QList<StoreInfo> &stores);

signals:
    void storeSelected(int storeId);
    void orderListRequested();
    void mypageRequested();
    void searchRequested();
    void logoutRequested();

private slots:
    void onCategoryClicked(const QString &category);
    void on_storeList_itemClicked(QListWidgetItem *item);
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navOrder_clicked();
    void on_navMypage_clicked();

private:
    Ui::HomeWidget *ui;
    NetworkManager *m_network;
    QString         m_userName;
    QString         m_currentCategory = "전체";

    void updateCategoryStyle(const QString &selectedCategory);
    void requestStoreList(const QString &category);
};
