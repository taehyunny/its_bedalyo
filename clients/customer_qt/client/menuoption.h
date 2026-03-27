#ifndef MENUOPTION_H
#define MENUOPTION_H

#include <QWidget>
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QMap>
#include "StoreDTO.h"    // OptionGroup, OptionItem 사용
#include "cartsession.h" // CartItemQt 사용

class NetworkManager;
namespace Ui { class menuoption; }

class menuoption : public QWidget {
    Q_OBJECT
public:
    explicit menuoption(NetworkManager *network, QWidget *parent = nullptr);
    ~menuoption();
    
    void loadMenuOption(int menuId, const QString &menuName, int basePrice);

signals:
    void backRequested();
    void selectedMenuFinished(CartItemQt item);
    void reviewRequested(int menuId);

private slots:
    void onMenuOptionDataReceived(int menuId, QList<OptionGroup> groups);
    void onIncreaseQty();
    void onDecreaseQty();
    void onAddToCart();
    void onBackClicked();

protected:
    // 🚀 마우스 클릭 이벤트를 감지하기 위한 함수 선언
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void buildOptionUI(const QList<OptionGroup> &groups);
    void clearOptionUI();
    void recalculatePrice();
    bool validateRequiredOptions();

    Ui::menuoption *ui;
    NetworkManager *m_network;

    int     m_menuId = 0;
    QString m_menuName;
    int     m_basePrice = 0;
    int     m_quantity = 1;

    // 필수 옵션 그룹 → 에러 라벨 매핑
    QMap<QButtonGroup*, QLabel*> m_errorLabels;
};


#endif