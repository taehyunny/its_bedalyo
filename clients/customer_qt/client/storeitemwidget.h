#pragma once
#include <QWidget>
#include <QLabel>
#include <QPixmap>

// ============================================================
// StoreItemWidget
// 가게 목록 카드 UI — 서버에서 데이터 받아와 동적으로 채우는 구조
//
// 사용법:
//   StoreItemWidget *w = new StoreItemWidget(parent);
//   w->setData(name, category, rating, reviewCount,
//              distanceKm, minTime, maxTime,
//              deliveryFee, minOrderAmount, tags);
//   w->setMainImage(pixmap);   // 서버 이미지 받은 후
//
// 레이아웃:
//   [ 큰 이미지 252x148 ][ 작은이미지1 ]
//                        [ 작은이미지2 ]
//   가게명                        25분
//   ⭐4.8(320) · 1.6km · 무료배달
//   최소주문 12,000원
//   [태그1] [태그2]
// ============================================================
class StoreItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit StoreItemWidget(QWidget *parent = nullptr);

    // 서버 데이터 수신 후 호출
    void setData(const QString &name,
                 const QString &category,
                 double rating, int reviewCount,
                 double distanceKm,
                 int minTime, int maxTime,
                 int deliveryFee,
                 int minOrderAmount,
                 const QStringList &tags = {});

    // 이미지 수신 후 교체
    void setMainImage(const QPixmap &pixmap);
    void setSubImage1(const QPixmap &pixmap);
    void setSubImage2(const QPixmap &pixmap);

    static int itemHeight() { return 260; }

private:
    // 이미지 라벨
    QLabel *m_mainImgLabel  = nullptr;
    QLabel *m_subImg1Label  = nullptr;
    QLabel *m_subImg2Label  = nullptr;

    // 텍스트 라벨
    QLabel *m_nameLabel     = nullptr;
    QLabel *m_timeLabel     = nullptr;
    QLabel *m_metaLabel     = nullptr;
    QLabel *m_minOrderLabel = nullptr;
    QWidget *m_tagContainer = nullptr;

    void buildLayout();

    static QString formatWon(int amount);
    static QString formatDeliveryFee(int fee);
    static QString formatDistance(double km);
    static QColor  placeholderColor(const QString &category);
};
