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

    // 주소 삭제 (addressdetailwidget에서 호출)
    void deleteAddress(int addressId);

signals:
    void backRequested();
    void addressSelected(const QString &address);
    void addressDetailRequested(const QString &roadAddr);   // 새 주소 설정
    void addressEditRequested(const AddressItem &item);     // 기존 주소 수정

private slots:
    void on_btnBack_clicked();
    void on_btnCurrentLocation_clicked();
    void on_btnAddHome_clicked();
    void on_btnAddWork_clicked();
    void onSearchTextChanged(const QString &text);
    void onSearchTimerTimeout();
    void onApiReplyFinished(QNetworkReply *reply);

public slots:
    void onAddressDetailCompleted(const AddressItem &item);

private:
    Ui::AddressWidget    *ui;
    NetworkManager       *m_network;
    QString               m_selectedAddress;
    QNetworkAccessManager *m_http;
    QTimer               *m_searchTimer;

    QList<AddressItem>    m_addressList;
    int                   m_nextLocalId = 1;

    void searchAddress(const QString &keyword);
    void buildSearchResults(const QJsonArray &results);
    void buildAddressList();
    void clearSearchResults();

    QWidget* makeAddressCard(const AddressItem &item);
    QWidget* makeSearchResultCard(const QString &roadAddr, const QString &jibunAddr);
};
