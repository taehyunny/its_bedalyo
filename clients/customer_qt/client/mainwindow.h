#pragma once
#include <QMainWindow>
#include "NetworkManager.h"
#include "config.h"
#include "loginwidget.h"
#include "homewidget.h"
#include "menucategori.h"
#include "searchwidget.h"
#include "searchresultwidget.h"
#include "orderhistorywidget.h"
#include "mypagewidget.h"
#include "storedetailwidget.h"
#include "policywidget.h"
#include "settingswidget.h"
#include "addresswidget.h"
#include "addressdetailwidget.h"
#include "cartwidget.h"
#include "menuoption.h"
#include "cartbarwidget.h"
#include "ordercompletewidget.h"
#include "form.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showLogin();
    void showHome();

private slots:
    void onLoginSuccess();
    void onLogoutRequested();
    void onCategorySelected(int categoryId, const QString &categoryName);
    void onBackToHome();
    void onSearchRequested();
    void onSearchExecuted(const QString &keyword);
    void onOrderListRequested();
    void onMypageRequested();
    void onFavoriteRequested();
    void onStoreSelected(int storeId);
    void onPolicyRequested();
    void onSettingsRequested();
    void onAddressRequested();
    void onAddressSelected(const QString &address);
    void onAddressDetailRequested(const QString &roadAddr);
    void onAddressEditRequested(const AddressItem &item);
    void onAddressDetailCompleted(const AddressItem &item);
    void onAddressDeleteRequested(int addressId);
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);
    void onCartRequested();
    void onCartClose();
    void onStoreDetailBack();
    void onOrderSuccess();
    
    // CartWidget이 데이터를 비우기 전에 가로챌 함수
    void onNetworkOrderCreated(int status, QString message, QString orderId);

private:
    Ui::MainWindow        *ui;
    NetworkManager        *m_network;
    LoginWidget           *m_loginWidget;
    HomeWidget            *m_homeWidget;
    menucategori          *m_menuWidget;
    SearchWidget          *m_searchWidget;
    SearchResultWidget    *m_searchResultWidget;
    OrderHistoryWidget    *m_orderHistoryWidget;
    MyPageWidget          *m_myPageWidget;
    StoreDetailWidget     *m_storeDetailWidget;
    PolicyWidget          *m_policyWidget;
    SettingsWidget        *m_settingsWidget;
    AddressWidget         *m_addressWidget;
    AddressDetailWidget   *m_addressDetailWidget;
    CartWidget            *m_cartWidget;
    menuoption            *m_menuOptionWidget;
    CartBarWidget         *m_cartBar = nullptr; // MainWindow가 직접 소유
    OrderCompleteWidget   *m_orderCompleteWidget;

    QList<CategoryInfoQt> m_cachedCategories;
    QWidget              *m_previousWidget = nullptr;

    Form                  *m_formWidget;

    // CartBar 위치 관련 헬퍼
    void repositionCartBar();
    void showCartBarForHome();  // 홈 화면용 (navBar 위에 배치)
    void showCartBarForStore(); // 가게상세용 (맨 아래 배치, navBar 없음)
};
