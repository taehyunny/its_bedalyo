#include "menuoption.h"
#include "ui_menuoption.h"
#include "NetworkManager.h"
#include <QRadioButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QDebug>
#include <QButtonGroup>
#include <QFrame>

menuoption::menuoption(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menuoption)
    , m_network(network)
{

    qRegisterMetaType<QList<OptionGroup>>("QList<OptionGroup>");
    
    ui->setupUi(this);
    ui->img_product_placeholder->hide();

    connect(ui->btn_plus,     &QPushButton::clicked, this, &menuoption::onIncreaseQty);
    connect(ui->btn_minus,    &QPushButton::clicked, this, &menuoption::onDecreaseQty);
    connect(ui->btn_back,     &QPushButton::clicked, this, &menuoption::onBackClicked);
    connect(ui->btn_add_cart, &QPushButton::clicked, this, &menuoption::onAddToCart);

    connect(m_network, &NetworkManager::onMenuOptionsReceived, this, &menuoption::onMenuOptionDataReceived);
}

menuoption::~menuoption() { delete ui; }

void menuoption::loadMenuOption(int menuId, const QString &menuName, int basePrice)
{
    m_menuId = menuId; m_menuName = menuName; m_basePrice = basePrice; m_quantity = 1;

    ui->lbl_title->setText(m_menuName);
    ui->lbl_price_value->setText(QString::number(m_basePrice) + "원");
    ui->lbl_quantity_value->setText("1");
    ui->btn_add_cart->setText(QString::number(m_basePrice) + "원 담기");

    clearOptionUI();
    // [39라인] NetworkManager에 추가한 함수 호출
    m_network->sendMenuOptionRequest(menuId);

}

void menuoption::buildOptionUI(const QList<OptionGroup> &groups)
{

    // 2. "불러오는 중..." 문구 숨기기 (이게 안 숨겨지면 화면이 안 바뀜)
    ui->lbl_option_placeholder->hide();

    if (groups.isEmpty()) {
        ui->lbl_option_placeholder->setText("선택 가능한 옵션이 없습니다.");
        ui->lbl_option_placeholder->show();
        return;
    }

    for (const auto &group : groups) {
        // 그룹 이름 (예: 매운맛 선택)
        QLabel *groupLabel = new QLabel(QString::fromStdString(group.groupName));
        groupLabel->setStyleSheet("font-weight: bold; font-size: 15px; margin-top: 15px; color: #333;");
        ui->dynamicOptionLayout->addWidget(groupLabel);

        // 필수 선택인 경우 라디오 버튼 그룹화
        QButtonGroup *btnGroup = group.isRequired ? new QButtonGroup(this) : nullptr;

        // [주의] group.options 인지 group.items 인지 확인 (제 가이드대로라면 options입니다)
        for (const auto &item : group.options) {
            QWidget *itemContainer = new QWidget();
            QHBoxLayout *itemLayout = new QHBoxLayout(itemContainer);
            itemLayout->setContentsMargins(10, 5, 10, 5);

            QAbstractButton *choiceBtn;
            if (group.isRequired) {
                choiceBtn = new QRadioButton(QString::fromStdString(item.optionName));
                if(btnGroup) btnGroup->addButton(choiceBtn, item.optionId);
            } else {
                choiceBtn = new QCheckBox(QString::fromStdString(item.optionName));
            }

            // 가격 및 ID 정보 저장
            choiceBtn->setProperty("price", item.additionalPrice);
            choiceBtn->setProperty("id", item.optionId);
            connect(choiceBtn, &QAbstractButton::clicked, this, &menuoption::recalculatePrice);

            itemLayout->addWidget(choiceBtn);
            itemLayout->addStretch();
            itemLayout->addWidget(new QLabel(item.additionalPrice > 0 ? "+" + QString::number(item.additionalPrice) + "원" : "0원"));

            ui->dynamicOptionLayout->addWidget(itemContainer);
        }
    }
    recalculatePrice(); // 합계 금액 업데이트
}

void menuoption::recalculatePrice()
{
    int optionTotal = 0;
    const auto buttons = this->findChildren<QAbstractButton *>();
    for (auto *btn : buttons) {
        // price property가 있는 버튼만 처리
        if (btn->isChecked() && btn->property("price").isValid()) {
            optionTotal += btn->property("price").toInt();
        }
    }
    int finalPrice = (m_basePrice + optionTotal) * m_quantity;
    ui->btn_add_cart->setText(QString::number(finalPrice) + "원 담기");
}

void menuoption::onIncreaseQty() { m_quantity++; ui->lbl_quantity_value->setText(QString::number(m_quantity)); recalculatePrice(); }
void menuoption::onDecreaseQty() { if (m_quantity > 1) { m_quantity--; ui->lbl_quantity_value->setText(QString::number(m_quantity)); recalculatePrice(); } }

void menuoption::onAddToCart()
{
    if (!validateRequiredOptions()) return;

    CartItemQt item;
    item.menuId   = m_menuId;
    item.menuName = m_menuName;
    item.quantity = m_quantity;
    
    int optTotal = 0;
    QStringList optionNames;
    const auto buttons = this->findChildren<QAbstractButton *>();
    for (auto *btn : buttons) {
        if (btn->isChecked() && btn->property("price").isValid()) {
            item.optionIds << btn->property("id").toInt();
            optTotal += btn->property("price").toInt();
            optionNames << btn->text().trimmed(); // 선택된 옵션명 수집
        }
    }
    item.unitPrice  = m_basePrice + optTotal;
    item.optionName = optionNames.join(" / "); // 예: "3단계 불타는 매운맛 / 곱빼기"
    
    emit selectedMenuFinished(item);
}

void menuoption::onBackClicked() { emit backRequested(); }

void menuoption::clearOptionUI()
{
    for (auto *g : findChildren<QButtonGroup*>()) {
        g->setParent(nullptr);
        delete g;
    }

    while (ui->dynamicOptionLayout->count() > 0) {
        QLayoutItem *item = ui->dynamicOptionLayout->takeAt(0);
        QWidget *w = item->widget();
        // lbl_option_placeholder는 ui가 관리하므로 절대 삭제하면 안됨!
        if (w && w != ui->lbl_option_placeholder) {
            w->setParent(nullptr);
            delete w;
        }
        delete item;
    }

    // placeholder는 레이아웃에서 빠졌으니 다시 추가하고 보여주기
    ui->dynamicOptionLayout->addWidget(ui->lbl_option_placeholder);
    ui->lbl_option_placeholder->setText("[ 옵션 정보를 불러오는 중... ]");
    ui->lbl_option_placeholder->show();
}

bool menuoption::validateRequiredOptions()
{
    const auto groups = this->findChildren<QButtonGroup *>();
    for (auto *g : groups) {
        if (g->checkedButton() == nullptr)
        {
            // 경고 메시지 띄우고 실패 반환
            QMessageBox::warning(this, "알림", "필수 옵션을 선택해주세요.");
            return false;
        }
    return true;
    }
}


void menuoption::onMenuOptionDataReceived(int menuId, QList<OptionGroup> groups)
{

    qDebug() << "[menuoption] 데이터 수신 시도 - 서버 menuId:" << menuId << " / 내 m_menuId:" << m_menuId;

    // 현재 열려 있는 메뉴 ID와 서버에서 보낸 메뉴 ID가 같은지 확인
    if (m_menuId != menuId) {
        qWarning() << "[menuoption] ID 불일치로 데이터 무시됨!";
        return; 
    }
        
    qDebug() << "[menuoption] ID 일치! 옵션 그리기 시작. 그룹 개수:" << groups.size();

    // 실제 UI를 생성하는 함수 호출
    buildOptionUI(groups);
}