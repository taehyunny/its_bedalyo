#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"
#include "OrderHistoryCard.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
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
    , m_formWidget(new Form(this))
    , m_deliveryCompleteWidget(new DeliveryCompleteWidget(m_network, this))
    , m_menureviewWidget(new menureview(m_network, this))
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
    ui->stackedWidget->addWidget(m_formWidget);
    ui->stackedWidget->addWidget(m_menureviewWidget);
    ui->stackedWidget->addWidget(m_deliveryCompleteWidget);

    // ── 로그인 / 로그아웃 ──
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(m_settingsWidget, &SettingsWidget::logoutRequested, this, &MainWindow::onLogoutRequested);

    // ── 홈 ──
    connect(m_homeWidget, &HomeWidget::categorySelected, this, &MainWindow::onCategorySelected);
    connect(m_homeWidget, &HomeWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_homeWidget, &HomeWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_homeWidget, &HomeWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_homeWidget, &HomeWidget::addressRequested, this, &MainWindow::onAddressRequested);
    connect(m_homeWidget, &HomeWidget::storeSelected, this, &MainWindow::onStoreSelected);
    connect(m_homeWidget, &HomeWidget::cartRequested, this, &MainWindow::onCartRequested);

    connect(m_network, &NetworkManager::onHeartbeatReceived,
            this, [this](QList<TopStoreInfoQt> topStores) {
                m_homeWidget->onMainHomeReceived(m_cachedCategories,
                                                 m_cachedBrands,
                                                 topStores);
            });

    // ── 카테고리 ──
    connect(m_menuWidget, &menucategori::backRequested, this, &MainWindow::onBackToHome);
    connect(m_menuWidget, &menucategori::storeSelected, this, &MainWindow::onStoreSelected);

    // ── 검색 ──
    connect(m_searchWidget, &SearchWidget::backRequested, this, &MainWindow::onBackToHome);
    connect(m_searchWidget, &SearchWidget::searchRequested, this, &MainWindow::onSearchExecuted);
    connect(m_searchWidget, &SearchWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_searchWidget, &SearchWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_searchWidget, &SearchWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_searchResultWidget, &SearchResultWidget::backRequested, this, &MainWindow::onSearchRequested);
    connect(m_searchResultWidget, &SearchResultWidget::storeSelected, this, &MainWindow::onStoreSelected);

    // ── 주문 내역 ──
    connect(m_orderHistoryWidget, &OrderHistoryWidget::homeRequested, this, &MainWindow::onBackToHome);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_orderHistoryWidget, &OrderHistoryWidget::mypageRequested, this, &MainWindow::onMypageRequested);
    connect(m_orderHistoryWidget->getReadyList(), &readylist::orderDetailRequested,
        this, [=](const QString& id) {
            m_formWidget->setCurrentOrderId(id);
            m_formWidget->updateOrderInfo(id, id, "상세 메뉴 내역...");
            ui->stackedWidget->setCurrentWidget(m_formWidget);
        });

    // ── 마이페이지 ──
    connect(m_myPageWidget, &MyPageWidget::homeRequested, this, &MainWindow::onBackToHome);
    connect(m_myPageWidget, &MyPageWidget::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_myPageWidget, &MyPageWidget::orderListRequested, this, &MainWindow::onOrderListRequested);
    connect(m_myPageWidget, &MyPageWidget::favoriteRequested, this, &MainWindow::onFavoriteRequested);
    connect(m_myPageWidget, &MyPageWidget::policyRequested, this, &MainWindow::onPolicyRequested);
    connect(m_myPageWidget, &MyPageWidget::settingsRequested, this, &MainWindow::onSettingsRequested);
    connect(m_myPageWidget, &MyPageWidget::addressRequested, this, &MainWindow::onAddressRequested);

    // ── 정책 / 설정 ──
    connect(m_policyWidget, &PolicyWidget::backRequested, this, &MainWindow::onMypageRequested);
    connect(m_settingsWidget, &SettingsWidget::backRequested, this, &MainWindow::onMypageRequested);

    // ── 주소 관리 ──
    connect(m_addressWidget, &AddressWidget::backRequested, this, [this]() {
        qDebug() << "[DEBUG] backRequested! m_prevWidget:" << m_prevWidget;
        if (m_prevWidget)
            ui->stackedWidget->setCurrentWidget(m_prevWidget);
        else
            ui->stackedWidget->setCurrentWidget(m_homeWidget);
        });
    connect(m_addressWidget, &AddressWidget::addressSelected, this, &MainWindow::onAddressSelected);
    connect(m_addressWidget, &AddressWidget::addressSelected, m_cartWidget, &CartWidget::onAddressUpdated);
    connect(m_addressWidget, &AddressWidget::addressDetailRequested, this, &MainWindow::onAddressDetailRequested);
    connect(m_addressWidget, &AddressWidget::addressEditRequested, this, &MainWindow::onAddressEditRequested);
    connect(m_addressDetailWidget, &AddressDetailWidget::backRequested,
        this, [this]() { ui->stackedWidget->setCurrentWidget(m_addressWidget); });
    connect(m_addressDetailWidget, &AddressDetailWidget::completed, this, &MainWindow::onAddressDetailCompleted);
    connect(m_addressDetailWidget, &AddressDetailWidget::completed,
        this, [this](const AddressItem& item) {
            m_cartWidget->onAddressUpdated(item.address);
        });
    connect(m_addressDetailWidget, &AddressDetailWidget::deleteRequested, this, &MainWindow::onAddressDeleteRequested);

    // ── 가게 상세 ──
    connect(m_storeDetailWidget, &StoreDetailWidget::backRequested, this, &MainWindow::onStoreDetailBack);
    connect(m_storeDetailWidget, &StoreDetailWidget::cartRequested, this, &MainWindow::onCartRequested);
    connect(m_storeDetailWidget, &StoreDetailWidget::menuSelected,
        this, [this](int menuId, QString menuName, int price) {
            m_menuOptionWidget->loadMenuOption(menuId, menuName, price);
            ui->stackedWidget->setCurrentWidget(m_menuOptionWidget);
        });

    // ── 메뉴 옵션 ──
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
            if (CartSession::instance().storeId == -1)
                CartSession::instance().storeId = m_storeDetailWidget->currentStoreId();
            if (!m_storeDetailWidget->currentStoreName().isEmpty())
                CartSession::instance().storeName = m_storeDetailWidget->currentStoreName();
            CartSession::instance().addItem(item);
            m_homeWidget->updateCartBar();
            m_storeDetailWidget->updateCartBar();
            ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
        });
    connect(m_menuOptionWidget, &menuoption::reviewRequested, this, [=](int menuId) {
        qDebug() << "메인윈도우: 리뷰 화면으로 전환합니다. 메뉴 ID:" << menuId;
        ui->stackedWidget->setCurrentWidget(m_menureviewWidget);
        m_menureviewWidget->loadReviews(menuId);  // ← 이 한 줄 추가
    });

    // ── 리뷰 ──
    connect(m_menureviewWidget, &menureview::backRequested, this, [=]() {
        qDebug() << "메인윈도우: 리뷰 화면에서 메뉴 옵션 화면으로 돌아갑니다.";
        ui->stackedWidget->setCurrentWidget(m_menuOptionWidget);
        });

    // ── 장바구니 ──
    connect(m_cartWidget, &CartWidget::closeRequested, this, &MainWindow::onCartClose);
    connect(m_cartWidget, &CartWidget::addMenuRequested,
        this, [this]() { ui->stackedWidget->setCurrentWidget(m_storeDetailWidget); });
    connect(m_cartWidget, &CartWidget::addressEditRequested, this, &MainWindow::onAddressRequested);
    connect(m_cartWidget, &CartWidget::orderSuccess, this, &MainWindow::onOrderSuccess);

    // ── 주문 생성 응답 (MainWindow 먼저, CartWidget 나중) ──
    disconnect(m_network, &NetworkManager::onOrderCreateReceived,
        m_cartWidget, &CartWidget::onOrderCreateReceived);
    connect(m_network, &NetworkManager::onOrderCreateReceived,
        this, &MainWindow::onNetworkOrderCreated);
    connect(m_network, &NetworkManager::onOrderCreateReceived,
        m_cartWidget, &CartWidget::onOrderCreateReceived);

    // ── 주문 상태 변경 알림 ──
    connect(m_network, &NetworkManager::onOrderStateChanged, this, [=](int state, QString orderId) {
        m_orderHistoryWidget->updateOrderState(orderId, state);

        // 현재 화면과 상관없이 항상 Form 업데이트
        if (m_formWidget->currentOrderId() == orderId) {
            m_formWidget->updateStatus(state);

            // state가 3이면 배달완료 화면으로 전환
            if (state == 3) {
                m_deliveryCompleteWidget->setOrderId(orderId);
                ui->stackedWidget->setCurrentWidget(m_deliveryCompleteWidget);
            }
        }

        if (state == 9) {
            m_orderHistoryWidget->moveToHistory(orderId, "배달거절");
        }
    });

    // ── Form 화면 ──
    connect(m_formWidget, &Form::backRequested, this, [=]() {
        ui->stackedWidget->setCurrentWidget(m_homeWidget);
        });

    // ── 배달 완료 ──
    connect(m_deliveryCompleteWidget, &DeliveryCompleteWidget::orderCompleted,
            this, [this](const QString &orderId) {
                m_orderHistoryWidget->getReadyList()->removeOrderCard(orderId);
            });

    connect(m_deliveryCompleteWidget, &DeliveryCompleteWidget::orderListRequested, this, [this]() {
        m_orderHistoryWidget->loadData();
        m_orderHistoryWidget->showPastOrdersTab();
        ui->stackedWidget->setCurrentWidget(m_orderHistoryWidget);
    });

    // ── 네트워크 ──
    connect(m_network, &NetworkManager::onMainHomeReceived, this, &MainWindow::onMainHomeReceived);
    connect(m_network, &NetworkManager::onDeliveryCompleteReceived, this, &MainWindow::handleDeliveryComplete);

    m_network->connectToServer(AppConfig::SERVER_IP, AppConfig::SERVER_PORT);
}

MainWindow::~MainWindow() { delete ui; }

// onLoginSuccess 슬롯 수정
void MainWindow::onLoginSuccess()
{
    m_homeWidget->setUserName(UserSession::instance().userName);
    m_homeWidget->setAddress(UserSession::instance().address);
    m_homeWidget->onMainHomeReceived(m_cachedCategories,
                                     m_cachedBrands,
                                     m_cachedTopStores);
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

void MainWindow::onMainHomeReceived(QList<CategoryInfoQt> categories,
                                    QStringList brandCategories,
                                    QList<TopStoreInfoQt> topStores)
{
    m_cachedCategories    = categories;
    m_cachedBrands        = brandCategories;  // ← 추가
    m_cachedTopStores     = topStores;        // ← 추가
}

void MainWindow::onCategorySelected(int categoryId, const QString& categoryName)
{
    m_menuWidget->setCategory(categoryId, categoryName, m_cachedCategories);
    ui->stackedWidget->setCurrentWidget(m_menuWidget);
}

void MainWindow::onSearchRequested()
{
    m_searchWidget->loadSearchData();
    ui->stackedWidget->setCurrentWidget(m_searchWidget);
}

void MainWindow::onSearchExecuted(const QString& keyword)
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

void MainWindow::onPolicyRequested() { ui->stackedWidget->setCurrentWidget(m_policyWidget); }
void MainWindow::onSettingsRequested() { ui->stackedWidget->setCurrentWidget(m_settingsWidget); }

void MainWindow::onAddressRequested()
{
    static int callCount = 0;
    qDebug() << "[DEBUG] onAddressRequested 호출 횟수:" << ++callCount
        << "현재 화면:" << ui->stackedWidget->currentWidget()->objectName();

    m_prevWidget = ui->stackedWidget->currentWidget();
    if (m_cartBar) m_cartBar->hide();
    m_addressWidget->loadData();
    ui->stackedWidget->setCurrentWidget(m_addressWidget);
}

void MainWindow::onAddressSelected(const QString& address)
{
    UserSession::instance().address = address;
    m_homeWidget->setAddress(address);
    m_cartWidget->onAddressUpdated(address);
    if (m_prevWidget)
        ui->stackedWidget->setCurrentWidget(m_prevWidget);
    else
        ui->stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::onAddressDetailRequested(const QString& roadAddr)
{
    m_addressDetailWidget->loadNewAddress(roadAddr);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

void MainWindow::onAddressEditRequested(const AddressItem& item)
{
    m_addressDetailWidget->loadEditAddress(item);
    ui->stackedWidget->setCurrentWidget(m_addressDetailWidget);
}

void MainWindow::onAddressDetailCompleted(const AddressItem& item)
{
    m_addressWidget->onAddressDetailCompleted(item);
    if (item.addressId <= 0 || item.isDefault) {
        UserSession::instance().address = item.address;
        m_homeWidget->setAddress(item.address);
        m_cartWidget->onAddressUpdated(item.address);
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
    m_prevWidget = ui->stackedWidget->currentWidget();
    m_storeDetailWidget->loadStoreData(storeId);
    m_storeDetailWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_storeDetailWidget);
}

void MainWindow::onStoreDetailBack()
{
    if (m_prevWidget == m_homeWidget)
        m_homeWidget->updateCartBar();
    else if (m_prevWidget == m_menuWidget)
        m_storeDetailWidget->updateCartBar();

    QWidget* target = m_prevWidget ? m_prevWidget : m_homeWidget;
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

void MainWindow::onNetworkOrderCreated(int status, QString message, QString orderId)
{
    Q_UNUSED(message)

        if (status == 200 || status == 0) {
            // 1. 메뉴 요약 텍스트
            QString menuSummary = "";
            if (!CartSession::instance().items.isEmpty()) {
                int extraCount = CartSession::instance().items.size() - 1;
                menuSummary = CartSession::instance().items[0].menuName +
                    (extraCount > 0 ? QString(" 외 %1건").arg(extraCount) : "");
            }

            QString storeName = CartSession::instance().storeName;
            // int     totalPrice = CartSession::instance().totalPrice();
            int total = CartSession::instance().totalWithDelivery;

            // 2. Form 화면에 데이터 세팅
            m_formWidget->setCurrentOrderId(orderId);
            m_formWidget->updateOrderInfo(storeName, orderId, menuSummary);
            m_formWidget->updateAddress(UserSession::instance().address);
            m_formWidget->updateStatus(0);
            m_formWidget->clearMenuItems();
            for (const CartItemQt& item : CartSession::instance().items) {
                m_formWidget->addMenuItem(item.menuName, item.quantity, item.unitPrice * item.quantity);
            }

            // 3. 주문 내역에 추가
            m_orderHistoryWidget->addPendingOrder(orderId, storeName, menuSummary, total);
        }
}

void MainWindow::onOrderSuccess()
{
    CartSession::instance().clear();
    m_homeWidget->updateCartBar();
    ui->stackedWidget->setCurrentWidget(m_formWidget);
}

void MainWindow::handleDeliveryComplete(const QString& orderId)
{
    qDebug() << "[MainWindow] 배달 완료 감지! 주문번호:" << orderId;
    ui->stackedWidget->setCurrentWidget(m_deliveryCompleteWidget);
}

void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome() { ui->stackedWidget->setCurrentWidget(m_homeWidget); }