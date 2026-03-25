#pragma once

#include <QWidget>
#include <QLayout>
#include <QDialog>
#include <QLabel>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class menucategori; }
QT_END_NAMESPACE

// ============================================================
// menucategori - 카테고리별 가게 목록 화면 (쿠팡이츠 스타일)
// ============================================================
class menucategori : public QWidget
{
    Q_OBJECT

public:
    explicit menucategori(NetworkManager *network, QWidget *parent = nullptr);
    ~menucategori();

    void setCategory(int categoryId, const QString &categoryName,
                     const QList<CategoryInfoQt> &categories);

signals:
    void backRequested();
    void storeSelected(int storeId);

private slots:
    void onCategoryTabClicked(int categoryId, const QString &categoryName);
    void onSortButtonClicked();
    void onStoreListReceived(QList<TopStoreInfoQt> stores);

private:
    Ui::menucategori *ui;
    NetworkManager   *m_network;

    int     m_currentCategoryId   = -1;
    QString m_currentCategoryName;
    QString m_currentSortLabel    = "추천순";

    void buildCategoryTabs(const QList<CategoryInfoQt> &categories);
    void updateStoreListUI(const QList<TopStoreInfoQt> &stores);
    void clearLayout(QLayout *layout);
    QWidget* makeStoreCard(const TopStoreInfoQt &store);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};
