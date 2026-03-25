#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new NetworkManager(this))
    , m_loginWidget(new LoginWidget(m_network, this))
    , m_homeWidget(new HomeWidget(m_network, this))
    , m_menuWidget(new menucategori(m_network, this))
    , m_searchWidget(new SearchWidget(m_network, this))
    , m_searchResultWidget(new SearchResultWidget(m_network, this))
    , m_orderHistoryWidget(new OrderHistoryWidget(m_network, this))
    , m_myPageWidget(new MyPageWidget(m_network, this))
    , m_storeDetailWidget(new StoreDetailWidget(m_network, this))
    , m_policyWidget(new PolicyWidget(this))
    , m_settingsWidget(new SettingsWidget(m_network, this))
    , m_addressWidget(new AddressWidget(m_network, this))
    , m_addressDetailWidget(new AddressDetailWidget(this))
    , m_cartWidget(new CartWidget(m_network, this))
    , m_menuOptionWidget(new menuoption(m_network, this))
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);
    setFixedSize(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    ui->stackedWidget->addWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_homeWidget);
    ui->stackedWidget->addWidget(m_menuWidget);
    ui->stackedWidget->addWidget(m_searchWidget);
    ui->stackedWidget->addWidget(m_searchResultWidget);
    ui->stackedWidget->addWidget(m_orderHistoryWidget);
    ui->stackedWidget->addWidget(m_myPageWidget);
    ui->stackedWidget->addWidget(m_storeDetailWidget);
    ui->stackedWidget->addWidget(m_policyWidget);
    ui->stackedWidget->addWidget(m_settingsWidget);
    ui->stackedWidget->addWidget(m_addressWidget);
    ui->stackedWidget->addWidget(m_addressDetailWidget);
    ui->stackedWidget->addWidget(m_menuOptionWidget);
    ui->stackedWidget->addWidget(m_cartWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);

    connect(m_loginWidget, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(m_homeWidget, &HomeWidget::categorySelected, this, &MainWindow::onCategorySelected);
    connect(m_menuWidget, &menucategori::backRequested, this, &MainWindow::onBackToHome);
    connect(m_homeWidget, &HomeWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_menuWidget, &menucategori::storeSelected, this, &MainWindow::onStoreSelected);
    connect(m_searchWidget, &SearchWidget::backRequested, this, &MainWindow::onBackToHome);
    connect(m_searchWidget, &SearchWidget::searchRequested, this, &MainWindow::onSearchExecuted);
    connect(m_searchResultWidget, &SearchResultWidget::backRequested, this, &MainWindow::onSearchRequested);
    connect(m_searchWidget, &SearchWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_searchWidget, &SearchWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_searchWidget, &SearchWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_homeWidget, &HomeWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::homeRequested, this, &MainWindow::onBackToHome);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_homeWidget, &HomeWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_myPageWidget, &MyPageWidget::homeRequested, this, &MainWindow::onBackToHome);
    connect(m_myPageWidget, &MyPageWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_myPageWidget, &MyPageWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_myPageWidget, &MyPageWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_myPageWidget, &MyPageWidget::policyRequested, this, &MainWindow::onPolicyRequested);
    connect(m_myPageWidget, &MyPageWidget::settingsRequested, this, &MainWindow::onSettingsRequested);
    connect(m_policyWidget, &PolicyWidget::backRequested, this, &MainWindow::onMypageRequested);
    connect(m_settingsWidget, &SettingsWidget::backRequested, this, &MainWindow::onMypageRequested);
    connect(m_settingsWidget, &SettingsWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
    connect(m_homeWidget, &HomeWidget::addressRequested, this, &MainWindow::onAddressRequested);
    connect(m_addressWidget, &AddressWidget::backRequested, this, &MainWindow::onBackToHome);
    connect(m_addressWidget, &AddressWidget::addressSelected, this, &MainWindow::onAddressSelected);
    connect(m_addressWidget, &AddressWidget::addressDetailRequested, this, &MainWindow::onAddressDetailRequested);
    connect(m_addressWidget, &AddressWidget::addressEditRequested, this, &MainWindow::onAddressEditRequested);
    connect(m_addressDetailWidget, &AddressDetailWidget::backRequested,
            this, [this]() { ui->stackedWidget->setCurrentWidget(m_addressWidget); });
    connect(m_addressDetailWidget, &AddressDetailWidget::completed, this, &MainWindow::onAddressDetailCompleted);
    connect(m_addressDetailWidget, &AddressDetailWidget::deleteRequested, this, &MainWindow::onAddressDeleteRequested);
    connect(m_homeWidget, &HomeWidget::storeSelected, this, &MainWindow::onStoreSelected);
    connect(m_storeDetailWidget, &StoreDetailWidget::backRequested, this, &MainWindow::onStoreDetailBack);
    connect(m_network, &NetworkManager::onMainHomeReceived, this, &MainWindow::onMainHomeReceived);

    // 메뉴 옵션 선택 → 장바구니 담기
    connect(m_storeDetailWidget, &StoreDetailWidget::menuSelected,
            this, [this](int menuId, QString menuName, int price) {
                m_menuOptionWidget->loadMenuOption(menuId, menuName, price);
                ui->stackedWidget->setCurrentWidget(m_menuOptionWidget);
            });
    connect(m_menuOptionWidget, &menuoption::backRequested, this, [this]() {
        ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
    });
    connect(m_menuOptionWidget, &menuoption::selectedMenuFinished,
            this, [this](CartItemQt item) {
                if (CartSession::instance().isFromDifferentStore(m_storeDetailWidget->currentStoreId())) {
                    auto reply = QMessageBox::question(this, "장바구니 초기화",
                        "다른 가게 메뉴가 담겨있습니다.\n장바구니를 비우고 담을까요?",
                        QMessageBox::Yes | QMessageBox::No);
                    if (reply != QMessageBox::Yes) return;
                    CartSession::instance().clear();
                }
                if (CartSession::instance().storeId == -1) {
                    CartSession::instance().storeId   = m_storeDetailWidget->currentStoreId();
                    CartSession::instance().storeName = m_storeDetailWidget->currentStoreName();
                }
                CartSession::instance().addItem(item);
                m_homeWidget->updateCartBar();
                m_storeDetailWidget->updateCartBar();
                ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
            });

    // CartBar 클릭 → 결제 화면 (홈 / 가게상세 각각)
    connect(m_homeWidget, &HomeWidget::cartRequested, this, &MainWindow::onCartRequested);
    connect(m_storeDetailWidget, &StoreDetailWidget::cartRequested, this, &MainWindow::onCartRequested);

    // 장바구니 화면
    connect(m_cartWidget, &CartWidget::closeRequested, this, &MainWindow::onCartClose);
    connect(m_cartWidget, &CartWidget::addMenuRequested,
            this, [this]() { ui->stackedWidget->setCurrentWidget(m_storeDetailWidget); });
    connect(m_cartWidget, &CartWidget::addressEditRequested, this, &MainWindow::onAddressRequested);
    connect(m_cartWidget, &CartWidget::orderSuccess, this, &MainWindow::onOrderSuccess);

    m_network->connectToServer(AppConfig::SERVER_IP, AppConfig::SERVER_PORT);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onLoginSuccess()
{
    m_homeWidget->setUserName(UserSession::instance().userName);
    m_homeWidget->setAddress(UserSession::instance().address);
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onLogoutRequested()
{
    UserSession::instance().clear();
    m_loginWidget->clearInputFields();
    CartSession::instance().clear();
    m_homeWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::onMainHomeReceived(QList<CategoryInfoQt> categories, QList<TopStoreInfoQt>)
{
    m_cachedCategories = categories;
}

void MainWindow::onCategorySelected(int categoryId, const QString &categoryName)
{
    m_menuWidget->setCategory(categoryId, categoryName, m_cachedCategories);
    ui->stackedWidget->setCurrentWidget(m_menuWidget);
}

void MainWindow::onSearchRequested()
{
    m_searchWidget->loadSearchData();
    ui->stackedWidget->setCurrentWidget(m_searchWidget);
}

void MainWindow::onSearchExecuted(const QString &keyword)
{
    m_searchResultWidget->search(keyword);
    ui->stackedWidget->setCurrentWidget(m_searchResultWidget);
}

void MainWindow::onOrderListRequested()
{
    m_orderHistoryWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_orderHistoryWidget);
}

void MainWindow::onMypageRequested()
{
    m_myPageWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_myPageWidget);
}

void MainWindow::onPolicyRequested()  { ui->stackedWidget->setCurrentWidget(m_policyWidget); }
void MainWindow::onSettingsRequested(){ ui->stackedWidget->setCurrentWidget(m_settingsWidget); }

void MainWindow::onAddressRequested()
{
    m_addressWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

void MainWindow::onAddressSelected(const QString &address)
{
    UserSession::instance().address = address;
    m_homeWidget->setAddress(address);
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onAddressDetailRequested(const QString &roadAddr)
{
    m_addressDetailWidget->loadNewAddress(roadAddr);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

void MainWindow::onAddressEditRequested(const AddressItem &item)
{
    m_addressDetailWidget->loadEditAddress(item);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

void MainWindow::onAddressDetailCompleted(const AddressItem &item)
{
    m_addressWidget->onAddressDetailCompleted(item);
    if (item.addressId <= 0 || item.isDefault) {
        UserSession::instance().address = item.address;
        m_homeWidget->setAddress(item.address);
    }
    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

void MainWindow::onAddressDeleteRequested(int addressId)
{
    m_addressWidget->deleteAddress(addressId);
    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

void MainWindow::onBackToHome()
{
    m_homeWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onFavoriteRequested() { /* TODO */ }

void MainWindow::onStoreSelected(int storeId)
{
    m_previousWidget = ui->stackedWidget->currentWidget();
    m_storeDetailWidget->loadStoreData(storeId);
    m_storeDetailWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}

void MainWindow::onStoreDetailBack()
{
    // 장바구니 바 갱신
    if (m_previousWidget == m_homeWidget)
        m_homeWidget->updateCartBar();
    else if (m_previousWidget == m_menuWidget)
        m_storeDetailWidget->updateCartBar(); // 카테고리에서 왔으면 cartBar 유지

    QWidget *target = m_previousWidget ? m_previousWidget : m_homeWidget;
    ui->stackedWidget->setCurrentWidget(target);
}

void MainWindow::onCartRequested()
{
    m_cartWidget->open();
    ui->stackedWidget->setCurrentWidget(m_cartWidget);
}

void MainWindow::onCartClose()
{
    m_storeDetailWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}

void MainWindow::onOrderSuccess()
{
    CartSession::instance().clear();
    m_homeWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
