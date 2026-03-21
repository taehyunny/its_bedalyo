#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QEvent>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SearchResultWidget; }
QT_END_NAMESPACE

// ============================================================
// SearchResultWidget - 검색 결과 화면
//
// [ 프로토콜 ]
// REQ_SEARCH_STORE = 2116 / RES_SEARCH_STORE = 2117
// ============================================================
class SearchResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchResultWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~SearchResultWidget();

    void search(const QString &keyword);

signals:
    void backRequested();
    void storeSelected(int storeId);

private slots:
    void on_btnBack_clicked();
    void onSearchResultReceived(QList<TopStoreInfoQt> stores);

private:
    Ui::SearchResultWidget *ui;
    NetworkManager         *m_network;
    QString                 m_keyword;

    void clearStoreList();
    void buildStoreList(const QList<TopStoreInfoQt> &stores);
    void showEmpty();

    // 카드 위젯 — 클릭은 내부 람다로 처리 (eventFilter 불필요)
    QWidget* makeStoreCard(const TopStoreInfoQt &store);

    static QString formatWon(int amount);
    static QString formatDeliveryFee(int fee);
    static QString placeholderColor(const QString &category);
    static QString categoryEmoji(const QString &category);
};
