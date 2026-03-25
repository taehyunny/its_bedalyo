#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "NetworkManager.h"
#include "cartbarwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StoreDetailWidget; }
QT_END_NAMESPACE

class StoreDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StoreDetailWidget(NetworkManager *network, QWidget *parent = nullptr);
    ~StoreDetailWidget();

    void loadStoreData(int storeId);
    void updateCartBar();

    int     currentStoreId()   const { return m_currentStoreId; }
    QString currentStoreName() const { return m_currentStoreName; }

signals:
    void backRequested();
    void menuSelected(int menuId, QString menuName, int price);
    void cartRequested(); // CartBar 클릭 → MainWindow가 결제 화면으로 전환

private slots:
    void on_btnBack_clicked();
    void onStoreDetailReceived(StoreDetailQt detail);
    void on_btnStoreInfo_clicked();
    void on_btnStoreInfoBack_clicked();
    void on_btnBackToMain_clicked();
    void on_btnRating_clicked();

private:
    Ui::StoreDetailWidget *ui;
    NetworkManager        *m_network;
    int                    m_currentStoreId = -1;
    QString                m_currentStoreName;
    StoreDetailQt         *m_lastStoreData = nullptr;

    void clearLayout(QLayout *layout);
    QWidget* makeMenuCard(const MenuQt &menu);

    QWidget* makeReviewCard(const ReviewQt &review);
    void renderReviews(const QList<ReviewQt> &reviews);
    void loadDummyReviews();

    void populateStoreInfoPage();
    QWidget* createInfoSection(const QString &icon, const QString &title);
    QWidget* createDataRow(const QString &title, const QString &content, bool showMapButton = false);

    void populatePhotoReviewBar(const QList<ReviewQt> &reviews);
    QWidget* createPhotoReviewCard(const ReviewQt &review);
};
