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
#include "cartsession.h"

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
    void updateCartBar();

signals:
    void categorySelected(int categoryId, const QString &categoryName);
    void storeSelected(int storeId);
    void searchRequested();
    void favoriteRequested();
    void orderListRequested();
    void mypageRequested();
    void logoutRequested();
    void cartRequested();
    void addressRequested();   // 주소 버튼 클릭 → 주소 관리 화면으로

private slots:
    void on_btnSearch_clicked();
    void on_btnAddress_clicked();  // 주소 버튼 클릭 슬롯
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();

    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

    void on_btnCartView_clicked();

private:
    Ui::HomeWidget *ui;
    NetworkManager *m_network;
    QString         m_userName;
    QString         m_address;

    QWidget* makeCategoryItem(int id, const QString &name, const QString &iconPath);
    QWidget* makeStoreCard(const TopStoreInfoQt &store);
};
