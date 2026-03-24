#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"
#include <QDebug>

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
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);
    setFixedSize(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->layout()->setSpacing(0);

    // ── 화면 등록 ──
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
    ui->stackedWidget->setCurrentWidget(m_loginWidget);

    // ── 로그인 ──
    connect(m_loginWidget, &LoginWidget::loginSuccess,
            this, &MainWindow::onLoginSuccess);

    // ── 카테고리 화면 ──
    connect(m_homeWidget, &HomeWidget::categorySelected,
            this, &MainWindow::onCategorySelected);
    connect(m_menuWidget, &menucategori::backRequested,
            this, &MainWindow::onBackToHome);

    // ── 검색 화면 ──
    connect(m_homeWidget, &HomeWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchWidget, &SearchWidget::backRequested,
            this, &MainWindow::onBackToHome);
    connect(m_searchWidget, &SearchWidget::searchRequested,
            this, &MainWindow::onSearchExecuted);
    connect(m_searchResultWidget, &SearchResultWidget::backRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchWidget, &SearchWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_searchWidget, &SearchWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);
    connect(m_searchWidget, &SearchWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);

    // ── 주문내역 화면 ──
    connect(m_homeWidget, &HomeWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::homeRequested,
            this, &MainWindow::onBackToHome);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);

    // ── 마이페이지 ──
    connect(m_homeWidget, &HomeWidget::mypageRequested,
            this, &MainWindow::onMypageRequested);
    connect(m_myPageWidget, &MyPageWidget::homeRequested,
            this, &MainWindow::onBackToHome);
    connect(m_myPageWidget, &MyPageWidget::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_myPageWidget, &MyPageWidget::orderListRequested,
            this, &MainWindow::onOrderListRequested);
    connect(m_myPageWidget, &MyPageWidget::favoriteRequested,
            this, &MainWindow::onFavoriteRequested);
    connect(m_myPageWidget, &MyPageWidget::policyRequested,
            this, &MainWindow::onPolicyRequested);
    connect(m_myPageWidget, &MyPageWidget::settingsRequested,
            this, &MainWindow::onSettingsRequested);

    // ── 약관 및 정책 ──
    connect(m_policyWidget, &PolicyWidget::backRequested,
            this, &MainWindow::onMypageRequested);

    // ── 설정 화면 ──
    connect(m_settingsWidget, &SettingsWidget::backRequested,
            this, &MainWindow::onMypageRequested);
    connect(m_settingsWidget, &SettingsWidget::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    // ── 주소 관리 화면 ──
    connect(m_homeWidget, &HomeWidget::addressRequested,
            this, &MainWindow::onAddressRequested);
    connect(m_addressWidget, &AddressWidget::backRequested,
            this, &MainWindow::onBackToHome);
    connect(m_addressWidget, &AddressWidget::addressSelected,
            this, &MainWindow::onAddressSelected);
    connect(m_addressWidget, &AddressWidget::addressDetailRequested,
            this, &MainWindow::onAddressDetailRequested);
    connect(m_addressWidget, &AddressWidget::addressEditRequested,
            this, &MainWindow::onAddressEditRequested);

    // ── 주소 설정 화면 ──
    connect(m_addressDetailWidget, &AddressDetailWidget::backRequested,
            this, [this]() { ui->stackedWidget->setCurrentWidget(m_addressWidget); });
    connect(m_addressDetailWidget, &AddressDetailWidget::completed,
            this, &MainWindow::onAddressDetailCompleted);
    connect(m_addressDetailWidget, &AddressDetailWidget::deleteRequested,
            this, &MainWindow::onAddressDeleteRequested);

    // ── 가게 상세 ──
    connect(m_homeWidget, &HomeWidget::storeSelected,
            this, &MainWindow::onStoreSelected);

    // ── 카테고리 데이터 캐싱 ──
    connect(m_network, &NetworkManager::onMainHomeReceived,
            this, &MainWindow::onMainHomeReceived);

    // ── 서버 연결 ──
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
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWindow::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                     QList<TopStoreInfoQt> /*topStores*/)
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

void MainWindow::onPolicyRequested()
{
    ui->stackedWidget->setCurrentWidget(m_policyWidget);
}

void MainWindow::onSettingsRequested()
{
    ui->stackedWidget->setCurrentWidget(m_settingsWidget);
}

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

// 새 주소 설정 화면
void MainWindow::onAddressDetailRequested(const QString &roadAddr)
{
    m_addressDetailWidget->loadNewAddress(roadAddr);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

// 기존 주소 수정 화면
void MainWindow::onAddressEditRequested(const AddressItem &item)
{
    m_addressDetailWidget->loadEditAddress(item);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

// 주소 설정 완료
void MainWindow::onAddressDetailCompleted(const AddressItem &item)
{
    m_addressWidget->onAddressDetailCompleted(item);

    if (item.isDefault) {
        UserSession::instance().address = item.address;
        m_homeWidget->setAddress(item.address);
    }

    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

// 주소 삭제
void MainWindow::onAddressDeleteRequested(int addressId)
{
    m_addressWidget->deleteAddress(addressId);
    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

void MainWindow::onBackToHome()
{
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onFavoriteRequested()
{
    // TODO: 즐겨찾기 화면
}

void MainWindow::onStoreSelected(int storeId)
{
    qDebug() << "[MainWindow] 가게 상세로 이동! storeId:" << storeId;
    m_storeDetailWidget->loadStoreData(storeId);
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
