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

struct CategoryInfo {
    int     categoryId = 0;
    QString name;
    QString imageUrl;
};

struct StoreInfo {
    int     storeId         = 0;
    QString name;
    QString category;
    double  rating          = 0.0;
    int     reviewCount     = 0;
    int     deliveryFee     = 0;
    int     minDeliveryTime = 0;
    int     maxDeliveryTime = 0;
    int     minOrderAmount  = 0;
    QString imageUrl;
};

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

    void populateCategories1(const QList<CategoryInfo> &categories);
    void populateCategories2(const QList<CategoryInfo> &categories);
    void populateStoreList(const QList<StoreInfo> &stores);

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

private:
    Ui::HomeWidget *ui;
    NetworkManager *m_network;
    QString         m_userName;
    QString         m_address;

    QWidget* makeCategoryItem(const CategoryInfo &cat);
    QWidget* makeStoreCard(const StoreInfo &store);

    static QString formatWon(int amount);
    static QString formatDeliveryFee(int fee);
    static QString formatTime(int min, int max);
    static QString placeholderColorForCard(const QString &category);
    static QString categoryEmoji(const QString &category);
};
