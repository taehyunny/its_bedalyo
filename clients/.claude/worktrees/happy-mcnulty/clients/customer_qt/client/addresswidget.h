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
// 로컬 주소 아이템
// ============================================================
struct AddressItem {
    int     addressId   = -1;
    QString address;
    QString detail;
    QString guide;
    QString label       = "기타";
    bool    isDefault   = false;
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
    void deleteAddress(int addressId);

signals:
    void backRequested();
    void addressSelected(const QString &address);
    void addressDetailRequested(const QString &roadAddr);
    void addressEditRequested(const AddressItem &item);

private slots:
    void on_btnBack_clicked();
    void on_btnCurrentLocation_clicked();
    void on_btnAddHome_clicked();
    void on_btnAddWork_clicked();
    void onSearchTextChanged(const QString &text);
    void onSearchTimerTimeout();
    void onApiReplyFinished(QNetworkReply *reply);

    // ── 서버 응답 슬롯 ──
    void onAddressListReceived(QList<AddressItemQt> addresses);
    void onAddressSaveReceived(int status, int addressId);
    void onAddressDeleteReceived(int status);
    void onAddressUpdateReceived(int status);
    void onAddressDefaultReceived(int status);

public slots:
    void onAddressDetailCompleted(const AddressItem &item);

private:
    Ui::AddressWidget    *ui;
    NetworkManager       *m_network;
    QString               m_selectedAddress;
    QNetworkAccessManager *m_http;
    QTimer               *m_searchTimer;

    QList<AddressItem>    m_addressList;
    int                   m_nextLocalId  = 1;
    bool                  m_serverLoaded = false;

    void searchAddress(const QString &keyword);
    void buildSearchResults(const QJsonArray &results);
    void buildAddressList();
    void clearSearchResults();

    QWidget* makeAddressCard(const AddressItem &item);
    QWidget* makeSearchResultCard(const QString &roadAddr, const QString &jibunAddr);
};
