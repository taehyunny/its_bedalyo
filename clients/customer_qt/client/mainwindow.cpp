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
    , m_menuOptionWidget(new menuoption(m_network, this)) // [추가] 객체 생성

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
    ui->stackedWidget->addWidget(m_menuOptionWidget); // [추가] 메뉴 옵션 화면 등록

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

    // ── 가게 상세 화면에서 뒤로가기 ──
    connect(m_storeDetailWidget, &StoreDetailWidget::backRequested,
            this, &MainWindow::onBackToHome);

    // ── 3페이지에서 메뉴 클릭 시 -> 4페이지(주문 옵션)로 이동할 준비 ──
    connect(m_storeDetailWidget, &StoreDetailWidget::menuSelected, 
            this, [this](int menuId, QString menuName, int price) {
                
        // 일단 클릭이 잘 되는지 터미널에 로그만 찍어둡니다.
        qDebug() << "[MainWindow] 메뉴 선택됨! 4페이지 연결 대기중 -> ID:" << menuId << "이름:" << menuName << "가격:" << price;
    });


    // ── 장바구니 ──
    connect(m_homeWidget, &HomeWidget::cartRequested,
            this, &MainWindow::onCartRequested);
    connect(m_storeDetailWidget, &StoreDetailWidget::menuSelected,
            this, [this](int menuId, QString menuName, int price) {
                CartItemQt item;
                item.menuId    = menuId;
                item.menuName  = menuName;
                item.unitPrice = price;
                item.quantity  = 1;

                // 다른 가게 메뉴면 경고 팝업
                if (CartSession::instance().isFromDifferentStore(m_storeDetailWidget->currentStoreId())) {
                    QMessageBox::StandardButton reply = QMessageBox::question(
                        this, "장바구니 초기화",
                        "다른 가게 메뉴가 담겨있습니다.\n장바구니를 비우고 담을까요?",
                        QMessageBox::Yes | QMessageBox::No
                        );
                    if (reply != QMessageBox::Yes) return;
                    CartSession::instance().clear();
                }

                if (CartSession::instance().storeId == -1) {
                    CartSession::instance().storeId   = m_storeDetailWidget->currentStoreId();
                    CartSession::instance().storeName = m_storeDetailWidget->currentStoreName();
                }

                CartSession::instance().addItem(item);
                m_storeDetailWidget->updateCartBar();
                qDebug() << "[MainWindow] 메뉴 담김:" << menuName << "총:" << CartSession::instance().totalCount();
            });

    connect(m_cartWidget, &CartWidget::closeRequested,
            this, &MainWindow::onCartClose);
    connect(m_cartWidget, &CartWidget::addMenuRequested,
            this, [this]() { ui->stackedWidget->setCurrentWidget(m_storeDetailWidget); });
    connect(m_cartWidget, &CartWidget::addressEditRequested,
            this, &MainWindow::onAddressRequested);
    connect(m_cartWidget, &CartWidget::orderSuccess,
            this, &MainWindow::onOrderSuccess);


    // ── 가게 상세 화면에서 메뉴 클릭 시 로직 수정 ──
    connect(m_storeDetailWidget, &StoreDetailWidget::menuSelected, 
            this, [this](int menuId, QString menuName, int price) {
        
        qDebug() << "[MainWindow] 메뉴 상세 페이지로 이동 -> " << menuName;

        // 1. 데이터를 menuoption 위젯에 로드
        m_menuOptionWidget->loadMenuOption(menuId, menuName, price);

        // 2. 화면을 menuoption 위젯으로 전환
        ui->stackedWidget->setCurrentWidget(m_menuOptionWidget);
    });

    // ── 메뉴 상세에서 뒤로가기 클릭 시 로직 추가 ──
    connect(m_menuOptionWidget, &menuoption::backRequested, this, [this]() {
        // 다시 가게 상세 화면(m_storeDetailWidget)으로 복귀
        ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
    });

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
    // 1. 세션 데이터 삭제
    UserSession::instance().clear();

    // 2. 로그인 위젯의 입력 필드 초기화 (추가된 부분)
    m_loginWidget->clearInputFields();

    // 3. 로그인 화면으로 전환
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

    bool isNewAddress = (item.addressId <= 0);
    if (isNewAddress || item.isDefault) {
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

void MainWindow::onCartRequested()
{
    m_cartWidget->open();
    ui->stackedWidget->setCurrentWidget(m_cartWidget);
}

void MainWindow::onCartClose()
{
    // 장바구니에서 나갈 때 이전 화면으로 (가게 상세 or 홈)
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}

void MainWindow::onOrderSuccess()
{
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
    m_homeWidget->updateCartBar();
}


void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
