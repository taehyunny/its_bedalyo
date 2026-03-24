#include "addresswidget.h"
#include "ui_addresswidget.h"
#include "UserSession.h"
#include "config.h"
#include <utility>
#include <QMessageBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QLabel>
#include <QPushButton>

AddressWidget::AddressWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddressWidget)
    , m_network(network)
    , m_http(new QNetworkAccessManager(this))
    , m_searchTimer(new QTimer(this))
{
    ui->setupUi(this);

    ui->btnCurrentLocation->setEnabled(false);

    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(500);

    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &AddressWidget::onSearchTextChanged);
    connect(m_searchTimer, &QTimer::timeout,
            this, &AddressWidget::onSearchTimerTimeout);
    connect(m_http, &QNetworkAccessManager::finished,
            this, &AddressWidget::onApiReplyFinished);
}

AddressWidget::~AddressWidget() { delete ui; }

// ============================================================
// 화면 진입 시 호출
// ============================================================
void AddressWidget::loadData()
{
    qDebug() << "[AddressWidget] loadData()";
    ui->searchEdit->clear();
    clearSearchResults();

    if (m_addressList.isEmpty()) {
        const QString curAddr = UserSession::instance().address;
        if (!curAddr.isEmpty()) {
            AddressItem item;
            item.addressId = m_nextLocalId++;
            item.address   = curAddr;
            item.label     = "기타";
            item.isDefault = true;
            m_addressList.append(item);
        }
    }

    buildAddressList();
}

// ============================================================
// 주소 설정 화면에서 완료 후 호출
// ============================================================
void AddressWidget::onAddressDetailCompleted(const AddressItem &item)
{
    bool found = false;
    for (int i = 0; i < m_addressList.size(); ++i) {
        if (m_addressList[i].addressId == item.addressId) {
            m_addressList[i] = item;
            found = true;
            break;
        }
    }
    if (!found) {
        AddressItem newItem = item;
        newItem.addressId = m_nextLocalId++;
        if (m_addressList.isEmpty()) {
            newItem.isDefault = true;
        } else {
            // 새 주소 추가 시 기존 isDefault 해제 후 새 주소를 기본으로
            for (auto &a : m_addressList) a.isDefault = false;
            newItem.isDefault = true;
        }
        m_addressList.append(newItem);
    }

    clearSearchResults();
    buildAddressList();
    ui->searchEdit->clear();
}

// ============================================================
// 주소 삭제 (addressdetailwidget에서 호출)
// ============================================================
void AddressWidget::deleteAddress(int addressId)
{
    m_addressList.removeIf([addressId](const AddressItem &a) {
        return a.addressId == addressId;
    });
    clearSearchResults();
    buildAddressList();
}

// ============================================================
// 저장된 주소 목록 빌드
// ============================================================
void AddressWidget::buildAddressList()
{
    for (const AddressItem &item : std::as_const(m_addressList))
        ui->addressListLayout->addWidget(makeAddressCard(item));
}

// ============================================================
// 주소 카드 위젯 (삭제 버튼 없음 — 연필 버튼으로 설정 화면 진입)
// ============================================================
QWidget* AddressWidget::makeAddressCard(const AddressItem &item)
{
    QWidget *card = new QWidget();
    card->setMinimumHeight(72);
    card->setObjectName("addrCard");
    card->setStyleSheet(
        item.isDefault
        ? "QWidget#addrCard { background:#f0f4ff; border-bottom:1px solid #d0dcff; }"
        : "QWidget#addrCard { background:#ffffff; border-bottom:1px solid #f0f0f0; }"
    );

    QHBoxLayout *hl = new QHBoxLayout(card);
    hl->setContentsMargins(20, 12, 12, 12);
    hl->setSpacing(10);

    // 아이콘
    QLabel *iconLabel = new QLabel(
        item.label == "집" ? "🏠" : item.label == "회사" ? "💼" : "📍"
    );
    iconLabel->setFixedWidth(24);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size:18px; background:transparent;");

    // 텍스트 영역
    QWidget *textWidget = new QWidget();
    textWidget->setStyleSheet("background:transparent;");
    QVBoxLayout *vl = new QVBoxLayout(textWidget);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);

    QHBoxLayout *labelRow = new QHBoxLayout();
    labelRow->setContentsMargins(0, 0, 0, 0);
    labelRow->setSpacing(6);

    QLabel *labelTag = new QLabel(item.label);
    labelTag->setStyleSheet("font-size:12px; color:#888888; background:transparent;");
    labelRow->addWidget(labelTag);

    if (item.isDefault) {
        QLabel *badge = new QLabel("선택됨");
        badge->setStyleSheet(
            "font-size:11px; color:#1565c0; font-weight:bold;"
            "background:#e0eaff; border-radius:4px; padding:1px 6px;"
        );
        labelRow->addWidget(badge);
    }
    labelRow->addStretch();

    QLabel *addrLabel = new QLabel(item.address);
    addrLabel->setStyleSheet("font-size:14px; color:#222222; background:transparent;");
    addrLabel->setWordWrap(true);

    vl->addLayout(labelRow);
    vl->addWidget(addrLabel);

    if (!item.detail.isEmpty()) {
        QLabel *detailLabel = new QLabel(item.detail);
        detailLabel->setStyleSheet("font-size:12px; color:#888888; background:transparent;");
        vl->addWidget(detailLabel);
    }

    hl->addWidget(iconLabel);
    hl->addWidget(textWidget, 1);

    // 연필 버튼 → 주소 설정 화면으로 (수정 모드)
    QPushButton *editBtn = new QPushButton("✏️");
    editBtn->setFixedSize(32, 32);
    editBtn->setStyleSheet(
        "QPushButton{background:transparent;border:none;font-size:14px;}"
        "QPushButton:hover{background:#f0f0f0;border-radius:16px;}"
    );
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, [this, item]() {
        emit addressEditRequested(item);
    });
    hl->addWidget(editBtn);

    // 카드 클릭 → 해당 주소 선택 (현재 배달 주소로 설정)
    QPushButton *clickOverlay = new QPushButton(card);
    clickOverlay->setStyleSheet(
        "QPushButton{background:transparent;border:none;}"
        "QPushButton:hover{background:rgba(0,0,0,0.02);}"
        "QPushButton:pressed{background:rgba(0,0,0,0.05);}"
    );
    clickOverlay->setCursor(Qt::PointingHandCursor);
    clickOverlay->lower();

    connect(clickOverlay, &QPushButton::clicked, this, [this, id = item.addressId, addr = item.address]() {
        for (auto &a : m_addressList)
            a.isDefault = (a.addressId == id);
        m_selectedAddress = addr;
        clearSearchResults();
        buildAddressList();
        qDebug() << "[AddressWidget] 주소 선택:" << addr;
        emit addressSelected(addr);
    });

    return card;
}

// ============================================================
// 검색창 입력 변경
// ============================================================
void AddressWidget::onSearchTextChanged(const QString &text)
{
    m_searchTimer->stop();

    if (text.trimmed().isEmpty()) {
        clearSearchResults();
        buildAddressList();
        return;
    }

    if (text.trimmed().length() >= 2)
        m_searchTimer->start();
}

void AddressWidget::onSearchTimerTimeout()
{
    searchAddress(ui->searchEdit->text().trimmed());
}

// ============================================================
// 도로명주소 API 호출
// ============================================================
void AddressWidget::searchAddress(const QString &keyword)
{
    qDebug() << "[AddressWidget] 주소 검색:" << keyword;

    QUrlQuery query;
    query.addQueryItem("confmKey", AppConfig::JUSO_API_KEY);
    query.addQueryItem("currentPage", "1");
    query.addQueryItem("countPerPage", "10");
    query.addQueryItem("keyword", keyword);
    query.addQueryItem("resultType", "json");

    QUrl url(AppConfig::JUSO_API_URL);
    url.setQuery(query);

    QNetworkRequest request(url);
    m_http->get(request);
}

// ============================================================
// API 응답 처리
// ============================================================
void AddressWidget::onApiReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[AddressWidget] API 오류:" << reply->errorString();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        qWarning() << "[AddressWidget] JSON 파싱 실패";
        return;
    }

    QJsonObject root    = doc.object();
    QJsonObject results = root["results"].toObject();
    QJsonArray  juso    = results["juso"].toArray();

    clearSearchResults();

    if (juso.isEmpty()) {
        QLabel *emptyLabel = new QLabel("검색 결과가 없습니다.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size:14px; color:#aaaaaa; padding:24px;");
        ui->addressListLayout->addWidget(emptyLabel);
        return;
    }

    buildSearchResults(juso);
}

// ============================================================
// 검색 결과 목록 빌드
// ============================================================
void AddressWidget::buildSearchResults(const QJsonArray &results)
{
    for (const QJsonValue &val : results) {
        QJsonObject obj   = val.toObject();
        QString roadAddr  = obj["roadAddr"].toString();
        QString jibunAddr = obj["jibunAddr"].toString();
        ui->addressListLayout->addWidget(makeSearchResultCard(roadAddr, jibunAddr));
    }
}

// ============================================================
// 검색 결과 카드 — 카드 전체가 클릭 영역
// ============================================================
QWidget* AddressWidget::makeSearchResultCard(const QString &roadAddr,
                                              const QString &jibunAddr)
{
    QPushButton *card = new QPushButton();
    card->setStyleSheet(
        "QPushButton{background:#ffffff;border:none;border-bottom:1px solid #f0f0f0;text-align:left;padding:0;}"
        "QPushButton:hover{background:#f5f8ff;}"
        "QPushButton:pressed{background:#e8f0ff;}"
    );
    card->setMinimumHeight(72);
    card->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *hl = new QHBoxLayout(card);
    hl->setContentsMargins(20, 12, 16, 12);
    hl->setSpacing(12);

    QLabel *iconLabel = new QLabel("📍");
    iconLabel->setFixedWidth(24);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size:18px; background:transparent;");
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QWidget *textWidget = new QWidget();
    textWidget->setStyleSheet("background:transparent;");
    textWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout *vl = new QVBoxLayout(textWidget);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(3);

    QLabel *roadLabel = new QLabel(roadAddr);
    roadLabel->setStyleSheet("font-size:14px; color:#222222; font-weight:bold; background:transparent;");
    roadLabel->setWordWrap(true);
    roadLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *jibunLabel = new QLabel(jibunAddr);
    jibunLabel->setStyleSheet("font-size:12px; color:#888888; background:transparent;");
    jibunLabel->setWordWrap(true);
    jibunLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    vl->addWidget(roadLabel);
    vl->addWidget(jibunLabel);

    hl->addWidget(iconLabel);
    hl->addWidget(textWidget, 1);

    connect(card, &QPushButton::clicked, this, [this, roadAddr]() {
        qDebug() << "[AddressWidget] 검색결과 선택 → 설정 화면:" << roadAddr;
        emit addressDetailRequested(roadAddr);
    });

    return card;
}

// ============================================================
// 결과 목록 초기화
// ============================================================
void AddressWidget::clearSearchResults()
{
    QLayout *layout = ui->addressListLayout;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}

// ============================================================
// 버튼 슬롯
// ============================================================
void AddressWidget::on_btnBack_clicked() { emit backRequested(); }

void AddressWidget::on_btnCurrentLocation_clicked()
{
    QMessageBox::information(this, "준비 중", "현재 위치 기능은 준비 중입니다.");
}

void AddressWidget::on_btnAddHome_clicked()  { emit addressDetailRequested(""); }
void AddressWidget::on_btnAddWork_clicked()  { emit addressDetailRequested(""); }
