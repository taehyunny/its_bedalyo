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
    
private:
    Ui::StoreDetailWidget *ui;
    NetworkManager *m_network;
    
    int m_currentStoreId; // 현재 보고 있는 가게 ID

    // void renderMenuList(const QList<MenuDTO>& menus);
    void clearLayout(QLayout* layout);
    QWidget* makeMenuCard(const MenuQt& menu);
};