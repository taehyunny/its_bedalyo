#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "NetworkManager.h"
#include "cartbarwidget.h"

// ============================================================
// DragScrollArea
// ============================================================
class DragScrollArea : public QScrollArea
{
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

QT_BEGIN_NAMESPACE
namespace Ui { class HomeWidget; }
QT_END_NAMESPACE

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~HomeWidget();

    void setUserName(const QString &userName);
    void setAddress(const QString &address);
    void updateCartBar(); // CartBar show/hide 갱신

signals:
    void addressRequested();
    void searchRequested();
    void categorySelected(int id, const QString &name);
    void storeSelected(int storeId);
    void orderListRequested();
    void mypageRequested();
    void favoriteRequested();
    void cartRequested(); // CartBar 클릭 → MainWindow가 받아서 결제 화면으로

private slots:
    void on_btnAddress_clicked();
    void on_btnSearch_clicked();
    void on_navHome_clicked();
    void on_navSearch_clicked();
    void on_navFavorite_clicked();
    void on_navOrder_clicked();
    void on_navMy_clicked();
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::HomeWidget   *ui;
    NetworkManager   *m_network;
    QString           m_userName;
    QString           m_address;

    QWidget* makeCategoryItem(int id, const QString &name, const QString &iconPath);
    QWidget* makeStoreCard(const TopStoreInfoQt &store);
};
