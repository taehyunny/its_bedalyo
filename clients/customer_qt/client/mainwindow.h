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
    void onAddressDetailRequested(const QString &roadAddr);  // 새 주소
    void onAddressEditRequested(const AddressItem &item);    // 기존 주소 수정
    void onAddressDetailCompleted(const AddressItem &item);
    void onAddressDeleteRequested(int addressId);

    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);
    void onCartRequested();
    void onCartClose();
    void onOrderSuccess();

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

    QList<CategoryInfoQt> m_cachedCategories;
};