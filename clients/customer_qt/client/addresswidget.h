#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonArray>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AddressWidget; }
QT_END_NAMESPACE

// ============================================================
// 로컬 주소 아이템 (서버 연동 전 메모리 관리용)
// ============================================================
struct AddressItem {
    int     addressId   = -1;       // 서버 PK (임시: -1)
    QString address;                // 도로명 주소
    QString detail;                 // 상세주소
    QString guide;                  // 길안내
    QString label       = "기타";  // 집 / 회사 / 기타
    bool    isDefault   = false;    // 현재 선택된 주소
};

// ============================================================
// AddressWidget - 주소 관리 화면
// ============================================================
class AddressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddressWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~AddressWidget();

    void loadData();
    QString selectedAddress() const { return m_selectedAddress; }

signals:
    void backRequested();
    void addressSelected(const QString &address);
    // 검색 결과 선택 → 주소 설정 화면으로
    void addressDetailRequested(const QString &roadAddr);

private slots:
    void on_btnBack_clicked();
    void on_btnCurrentLocation_clicked();
    void on_btnAddHome_clicked();
    void on_btnAddWork_clicked();
    void onSearchTextChanged(const QString &text);
    void onSearchTimerTimeout();
    void onApiReplyFinished(QNetworkReply *reply);

public slots:
    // 주소 설정 화면에서 완료 후 호출
    void onAddressDetailCompleted(const AddressItem &item);

private:
    Ui::AddressWidget    *ui;
    NetworkManager       *m_network;
    QString               m_selectedAddress;
    QNetworkAccessManager *m_http;
    QTimer               *m_searchTimer;

    QList<AddressItem>    m_addressList;   // 로컬 주소 목록
    int                   m_nextLocalId = 1; // 임시 ID 증가용

    void searchAddress(const QString &keyword);
    void buildSearchResults(const QJsonArray &results);
    void buildAddressList();
    void clearSearchResults();

    // 주소 카드 (저장된 주소)
    QWidget* makeAddressCard(const AddressItem &item);
    // 검색 결과 카드
    QWidget* makeSearchResultCard(const QString &roadAddr, const QString &jibunAddr);
};
