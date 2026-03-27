#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UserSession.h"
#include "OrderHistoryCard.h"
#include <QDebug>
#include <QMessageBox>


#include <QTimer>  // [임시 테스트용] 타이머 기능 추가! - 배달완료용

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
    , m_orderCompleteWidget(new OrderCompleteWidget(m_network, this))
    , m_formWidget(new Form(this))
    , m_deliveryCompleteWidget(new DeliveryCompleteWidget(m_network, this))
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
    ui->stackedWidget->addWidget(m_orderCompleteWidget);
    ui->stackedWidget->setCurrentWidget(m_loginWidget);
    ui->stackedWidget->addWidget(m_formWidget);
    ui->stackedWidget->addWidget(m_deliveryCompleteWidget);

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

    connect(m_network, &NetworkManager::onOrderStateChanged, this, [=](int state, QString orderId) {
        // 🚀 포인트: 지금 사용자가 보고 있는 상세화면의 주문번호가 알림이 온 번호와 같을 때만 업데이트!
        m_orderHistoryWidget->updateOrderState(orderId, state);
        if (ui->stackedWidget->currentWidget() == m_formWidget &&
            m_formWidget->currentOrderId() == orderId) {
            m_formWidget->updateStatus(state);

        }
    });

    connect(m_orderHistoryWidget->getReadyList(), &readylist::orderDetailRequested, this, [=](const QString &id){

        // 🚀 중요: 폼 위젯에 클릭한 주문의 ID를 먼저 저장합니다.
        m_formWidget->setCurrentOrderId(id);

        m_formWidget->updateOrderInfo(id, id, "상세 메뉴 내역...");
        ui->stackedWidget->setCurrentWidget(m_formWidget);
    });

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
                // storeId가 -1일 때뿐 아니라 항상 storeName 동기화
                if (CartSession::instance().storeId == -1)
                    CartSession::instance().storeId = m_storeDetailWidget->currentStoreId();

                CartSession::instance().storeName = m_storeDetailWidget->currentStoreName();
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

    // 기존 연결을 끊습니다 (Qt 매크로 방식을 사용하여 private 접근 제한을 우회합니다)
    disconnect(m_network, SIGNAL(onOrderCreateReceived(int,QString,QString)),
               m_cartWidget, SLOT(onOrderCreateReceived(int,QString,QString)));

    // 메인 화면이 신호를 가장 먼저 가로채도록 연결합니다
    connect(m_network, &NetworkManager::onOrderCreateReceived,
            this, &MainWindow::onNetworkOrderCreated);

    // 장바구니 화면이 그다음으로 신호를 받도록 다시 연결해 줍니다 (private 접근 제한 우회)
    connect(m_network, SIGNAL(onOrderCreateReceived(int,QString,QString)),
            m_cartWidget, SLOT(onOrderCreateReceived(int,QString,QString)));

    // 주문 내역에서 상세 메뉴 요청 시 → 주문 완료 화면으로 전환
    connect(m_orderHistoryWidget->getReadyList(), &readylist::orderDetailRequested, this, [=](QString id){
        m_formWidget->updateOrderInfo(id, "ORD-001", "상세 메뉴 내역..."); // form 데이터 채우기
        ui->stackedWidget->setCurrentWidget(m_formWidget); // form.ui 화면으로 전환
    });

    //  배달 완료 화면에서 '등록하기'를 누르면 주문 내역 화면으로 이동
    connect(m_deliveryCompleteWidget, &DeliveryCompleteWidget::orderListRequested, this, [this]() {

        // 화면 갱신 및 탭 이동
        m_orderHistoryWidget->loadData();
        m_orderHistoryWidget->showPastOrdersTab();
        ui->stackedWidget->setCurrentWidget(m_orderHistoryWidget);
    });

    m_network->connectToServer(AppConfig::SERVER_IP, AppConfig::SERVER_PORT);

    connect(m_network, &NetworkManager::onDeliveryCompleteReceived, this, &MainWindow::handleDeliveryComplete);
    // [임시 테스트용] 프로그램 실행 1초(1000ms) 뒤에 강제로 배달 완료 함수를 실행합니다
    // TODO: UI 작업 다 끝나면 이 부분은 꼭 지워주세요!!!
    QTimer::singleShot(1000, this, [this]() {
        qDebug() << "[테스트] 1초 경과! 배달 완료 창을 강제로 엽니다.";
        handleDeliveryComplete("TEST_ORDER_999"); // 우리가 예전에 만들어둔 그 함수를 강제 호출
    });
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

// 신규 슬롯: CartWidget이 세션을 비우기 전에 실행되어 데이터를 안전하게 보존합니다.
void MainWindow::onNetworkOrderCreated(int status, QString message, QString orderId)
{
    Q_UNUSED(message)

    // 주의: 서버 응답 코드가 0(성공)이거나 200(성공)일 때 모두 처리되도록 수정
    if (status == 200 || status == 0) {

        // --- 1. 주문 완료 화면에 데이터 세팅 ---
        m_orderCompleteWidget->clearMenuItems();
        m_orderCompleteWidget->setOrderData(
            orderId, // 가짜 "ORD-001" 대신 서버가 준 진짜 주문번호 사용!
            CartSession::instance().storeName,
            UserSession::instance().address
            );
        for (const CartItemQt &item : CartSession::instance().items) {
            m_orderCompleteWidget->addMenuItem(item.quantity, item.menuName);
        }

        // --- 2. 주문 내역(준비중) 목록에 데이터 세팅 ---
        QString storeName = CartSession::instance().storeName;
        int totalPrice = CartSession::instance().totalPrice();
        QString menuSummary = "";
        if(!CartSession::instance().items.isEmpty()) {
            int extraCount = CartSession::instance().items.size() - 1;
            menuSummary = CartSession::instance().items[0].menuName +
                          (extraCount > 0 ? QString(" 외 %1건").arg(extraCount) : "");
        }

        // 진짜 orderId와 안전하게 빼둔 데이터를 사용해 목록에 추가!
        m_orderHistoryWidget->addPendingOrder(orderId, storeName, menuSummary, totalPrice);
    }
}

void MainWindow::onOrderSuccess()
{
    // 장바구니 비우기 (이제 데이터가 날아가도 상관없음!)
    CartSession::instance().clear();
    m_homeWidget->updateCartBar();

    // 완료 화면으로 이동
    ui->stackedWidget->setCurrentWidget(m_orderCompleteWidget);
}

void MainWindow::handleDeliveryComplete(const QString &orderId)
{
    qDebug() << "[MainWindow] 픽업/배달 완료 감지! 화면 전환 시작. 주문번호:" << orderId;

    ui->stackedWidget->setCurrentWidget(m_deliveryCompleteWidget);
}


void MainWindow::showLogin() { ui->stackedWidget->setCurrentWidget(m_loginWidget); }
void MainWindow::showHome()  { ui->stackedWidget->setCurrentWidget(m_homeWidget); }
