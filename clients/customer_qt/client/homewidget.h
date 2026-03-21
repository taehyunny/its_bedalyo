#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HomeWidget; }
QT_END_NAMESPACE

class DragScrollArea : public QScrollArea {
    Q_OBJECT
public:
    explicit DragScrollArea(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    bool   m_dragging = false;
    QPoint m_lastPos;
};

class HomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit HomeWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~HomeWidget();

    void setUserName(const QString &userName);
    void setAddress(const QString &address);

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void categorySelected(int categoryId, const QString &categoryName);
    void storeSelected(int storeId);
    void searchRequested();
    void favoriteRequested();
    void orderListRequested();
    void mypageRequested();
    void logoutRequested();

private slots:
    void on_btnSearch_clicked();
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

    // 서버에서 메인 홈 데이터 수신 시 호출
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

private:
    Ui::HomeWidget *ui;
    NetworkManager *m_network;
    QString         m_userName;
    QString         m_address;

    // 카테고리 아이템 위젯 생성
    QWidget* makeCategoryItem(int id, const QString &name, const QString &iconPath);

    // 가게 카드 위젯 생성
    QWidget* makeStoreCard(const TopStoreInfoQt &store);

    static QString formatWon(int amount);
    static QString formatDeliveryFee(int fee);
    static QString placeholderColorForCard(const QString &category);
    static QString categoryEmoji(const QString &category);
};
