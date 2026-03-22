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

signals:
    void categorySelected(int categoryId, const QString &categoryName);
    void storeSelected(int storeId);
    void searchRequested();
    void favoriteRequested();
    void orderListRequested();
    void mypageRequested();
    void logoutRequested();
    void cartRequested();          // 카트 바 클릭 → 장바구니 화면으로

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

    // 카트 바 클릭 슬롯
    void on_btnCartView_clicked();

private:
    Ui::HomeWidget *ui;
    NetworkManager *m_network;
    QString         m_userName;
    QString         m_address;

    // 카테고리 아이템 위젯 생성
    QWidget* makeCategoryItem(int id, const QString &name, const QString &iconPath);

    // 가게 카드 위젯 생성
    QWidget* makeStoreCard(const TopStoreInfoQt &store);

    // ── 카트 바 UI 갱신 ──
    // CartSession 상태를 읽어 카트 바를 show/hide하고 수량·금액 업데이트
    void updateCartBar();

};
