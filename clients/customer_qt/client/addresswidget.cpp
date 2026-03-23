#include "addresswidget.h"
#include "ui_addresswidget.h"
#include "UserSession.h"
#include "config.h"
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

    // 로컬 목록이 비어있으면 UserSession 주소로 초기화
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
    // 기존 목록에 있으면 수정, 없으면 추가
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
        // 첫 주소면 기본으로
        if (m_addressList.isEmpty()) newItem.isDefault = true;
        m_addressList.append(newItem);
    }

    clearSearchResults();
    buildAddressList();
    ui->searchEdit->clear();
}

// ============================================================
// 저장된 주소 목록 빌드
// ============================================================
void AddressWidget::buildAddressList()
{
    for (const AddressItem &item : m_addressList)
        ui->addressListLayout->addWidget(makeAddressCard(item));
}

// ============================================================
// 주소 카드 위젯 (저장된 주소)
// ============================================================
QWidget* AddressWidget::makeAddressCard(const AddressItem &item)
{
    QWidget *card = new QWidget();
    card->setMinimumHeight(72);
    card->setStyleSheet(
        item.isDefault
        ? "QWidget#addrCard { background:#f0f4ff; border-bottom:1px solid #d0dcff; }"
        : "QWidget#addrCard { background:#ffffff; border-bottom:1px solid #f0f0f0; }"
    );
    card->setObjectName("addrCard");

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

    // 텍스트
    QWidget *textWidget = new QWidget();
    textWidget->setStyleSheet("background:transparent;");
    QVBoxLayout *vl = new QVBoxLayout(textWidget);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);

    // 라벨 + 현재 선택 뱃지
    QHBoxLayout *labelRow = new QHBoxLayout();
    labelRow->setContentsMargins(0, 0, 0, 0);
    labelRow->setSpacing(6);

    QLabel *labelTag = new QLabel(item.label);
    labelTag->setStyleSheet("font-size:12px; color:#888888; background:transparent;");

    if (item.isDefault) {
        QLabel *badge = new QLabel("선택됨");
        badge->setStyleSheet(
            "font-size:11px; color:#1565c0; font-weight:bold;"
            "background:#e0eaff; border-radius:4px; padding:1px 6px;"
        );
        labelRow->addWidget(labelTag);
        labelRow->addWidget(badge);
        labelRow->addStretch();
    } else {
        labelRow->addWidget(labelTag);
        labelRow->addStretch();
    }

    QLabel *addrLabel = new QLabel(item.address);
    addrLabel->setStyleSheet("font-size:14px; color:#222222; background:transparent;");
    addrLabel->setWordWrap(true);

    if (!item.detail.isEmpty()) {
        QLabel *detailLabel = new QLabel(item.detail);
        detailLabel->setStyleSheet("font-size:12px; color:#888888; background:transparent;");
        vl->addLayout(labelRow);
        vl->addWidget(addrLabel);
        vl->addWidget(detailLabel);
    } else {
        vl->addLayout(labelRow);
        vl->addWidget(addrLabel);
    }

    hl->addWidget(iconLabel);
    hl->addWidget(textWidget, 1);

    // 연필 버튼 (수정)
    QPushButton *editBtn = new QPushButton("✏️");
    editBtn->setFixedSize(32, 32);
    editBtn->setStyleSheet(
        "QPushButton{background:transparent;border:none;font-size:14px;}"
        "QPushButton:hover{background:#f0f0f0;border-radius:16px;}"
    );
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, [this, item]() {
        emit addressDetailRequested(item.address);
        // TODO: 수정 모드로 addressdetailwidget 열기
    });
    hl->addWidget(editBtn);

    // 삭제 버튼 (현재 선택된 주소는 표시 안 함)
    if (!item.isDefault) {
        QPushButton *delBtn = new QPushButton("✕");
        delBtn->setFixedSize(32, 32);
        delBtn->setStyleSheet(
            "QPushButton{background:transparent;border:none;font-size:14px;color:#bbbbbb;}"
            "QPushButton:hover{color:#e53935;background:#fff0f0;border-radius:16px;}"
        );
        delBtn->setCursor(Qt::PointingHandCursor);
        connect(delBtn, &QPushButton::clicked, this, [this, id = item.addressId]() {
            m_addressList.removeIf([id](const AddressItem &a) {
                return a.addressId == id;
            });
            clearSearchResults();
            buildAddressList();
        });
        hl->addWidget(delBtn);
    }

    // 카드 클릭 → 해당 주소 선택 (현재 배달 주소로 설정)
    QPushButton *clickOverlay = new QPushButton(card);
    clickOverlay->setStyleSheet(
        "QPushButton{background:transparent;border:none;}"
        "QPushButton:hover{background:rgba(0,0,0,0.02);}"
        "QPushButton:pressed{background:rgba(0,0,0,0.05);}"
    );
    clickOverlay->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *overlayLayout = new QVBoxLayout(card);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->addWidget(clickOverlay);
    clickOverlay->lower(); // 버튼들 뒤로
    clickOverlay->stackUnder(iconLabel);

    connect(clickOverlay, &QPushButton::clicked, this, [this, id = item.addressId, addr = item.address]() {
        // 선택된 주소 변경
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
// 검색 결과 카드 — 클릭 시 주소 설정 화면으로
// ============================================================
QWidget* AddressWidget::makeSearchResultCard(const QString &roadAddr,
                                              const QString &jibunAddr)
{
    QWidget *card = new QWidget();
    card->setStyleSheet("QWidget { background:#ffffff; border-bottom:1px solid #f0f0f0; }");
    card->setMinimumHeight(68);
    card->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *hl = new QHBoxLayout(card);
    hl->setContentsMargins(20, 12, 12, 12);
    hl->setSpacing(12);

    QLabel *iconLabel = new QLabel("📍");
    iconLabel->setFixedWidth(24);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size:18px; background:transparent;");

    QWidget *textWidget = new QWidget();
    textWidget->setStyleSheet("background:transparent;");
    QVBoxLayout *vl = new QVBoxLayout(textWidget);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);

    QLabel *roadLabel = new QLabel(roadAddr);
    roadLabel->setStyleSheet("font-size:14px; color:#222222; font-weight:bold; background:transparent;");
    roadLabel->setWordWrap(true);

    QLabel *jibunLabel = new QLabel(jibunAddr);
    jibunLabel->setStyleSheet("font-size:12px; color:#888888; background:transparent;");
    jibunLabel->setWordWrap(true);

    vl->addWidget(roadLabel);
    vl->addWidget(jibunLabel);

    hl->addWidget(iconLabel);
    hl->addWidget(textWidget, 1);

    // 카드 전체 클릭 → 주소 설정 화면으로
    QPushButton *clickBtn = new QPushButton(card);
    clickBtn->setStyleSheet(
        "QPushButton{background:transparent;border:none;}"
        "QPushButton:hover{background:rgba(0,0,0,0.03);}"
        "QPushButton:pressed{background:rgba(0,0,0,0.07);}"
    );
    clickBtn->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *overlayLayout = new QVBoxLayout(card);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->addWidget(clickBtn);
    clickBtn->raise();

    connect(clickBtn, &QPushButton::clicked, this, [this, roadAddr]() {
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

void AddressWidget::on_btnAddHome_clicked()
{
    emit addressDetailRequested("");
}

void AddressWidget::on_btnAddWork_clicked()
{
    emit addressDetailRequested("");
}
