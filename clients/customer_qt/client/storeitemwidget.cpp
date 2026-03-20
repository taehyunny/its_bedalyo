#include "StoreItemWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLocale>

// ============================================================
// 생성자 — 레이아웃 뼈대만 잡음, 데이터는 setData()로 주입
// ============================================================
StoreItemWidget::StoreItemWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(itemHeight());
    setStyleSheet("background-color: #ffffff;");
    buildLayout();
}

void StoreItemWidget::buildLayout()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── 이미지 영역 ──
    QWidget *imgArea = new QWidget();
    imgArea->setFixedHeight(148);

    QHBoxLayout *imgLayout = new QHBoxLayout(imgArea);
    imgLayout->setContentsMargins(0, 0, 0, 0);
    imgLayout->setSpacing(2);

    m_mainImgLabel = new QLabel();
    m_mainImgLabel->setFixedSize(252, 148);
    m_mainImgLabel->setAlignment(Qt::AlignCenter);
    m_mainImgLabel->setScaledContents(true);
    m_mainImgLabel->setStyleSheet("background-color: #eeeeee;");

    QVBoxLayout *subLayout = new QVBoxLayout();
    subLayout->setContentsMargins(0, 0, 0, 0);
    subLayout->setSpacing(2);

    m_subImg1Label = new QLabel();
    m_subImg1Label->setFixedSize(134, 73);
    m_subImg1Label->setAlignment(Qt::AlignCenter);
    m_subImg1Label->setScaledContents(true);
    m_subImg1Label->setStyleSheet("background-color: #e4e4e4;");

    m_subImg2Label = new QLabel();
    m_subImg2Label->setFixedSize(134, 73);
    m_subImg2Label->setAlignment(Qt::AlignCenter);
    m_subImg2Label->setScaledContents(true);
    m_subImg2Label->setStyleSheet("background-color: #d8d8d8;");

    subLayout->addWidget(m_subImg1Label);
    subLayout->addWidget(m_subImg2Label);
    imgLayout->addWidget(m_mainImgLabel);
    imgLayout->addLayout(subLayout);
    root->addWidget(imgArea);

    // ── 텍스트 정보 영역 ──
    QWidget *infoArea = new QWidget();
    infoArea->setStyleSheet("background: #ffffff;");

    QVBoxLayout *infoLayout = new QVBoxLayout(infoArea);
    infoLayout->setContentsMargins(14, 10, 14, 8);
    infoLayout->setSpacing(4);

    // 가게명 + 배달시간
    QHBoxLayout *nameRow = new QHBoxLayout();
    nameRow->setContentsMargins(0, 0, 0, 0);

    m_nameLabel = new QLabel("-");
    m_nameLabel->setStyleSheet("font-size:16px; font-weight:bold; color:#111111;");

    m_timeLabel = new QLabel("-");
    m_timeLabel->setStyleSheet("font-size:14px; color:#333333;");
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    nameRow->addWidget(m_nameLabel);
    nameRow->addStretch();
    nameRow->addWidget(m_timeLabel);
    infoLayout->addLayout(nameRow);

    // 별점 · 거리 · 배달비
    m_metaLabel = new QLabel("-");
    m_metaLabel->setTextFormat(Qt::RichText);
    m_metaLabel->setStyleSheet("font-size:12px; color:#555555;");
    infoLayout->addWidget(m_metaLabel);

    // 최소주문
    m_minOrderLabel = new QLabel("-");
    m_minOrderLabel->setStyleSheet("font-size:12px; color:#888888;");
    infoLayout->addWidget(m_minOrderLabel);

    // 태그 컨테이너 (동적으로 뱃지 추가)
    m_tagContainer = new QWidget();
    m_tagContainer->setStyleSheet("background: transparent;");
    QHBoxLayout *tagLayout = new QHBoxLayout(m_tagContainer);
    tagLayout->setContentsMargins(0, 2, 0, 0);
    tagLayout->setSpacing(6);
    tagLayout->addStretch();
    infoLayout->addWidget(m_tagContainer);

    root->addWidget(infoArea);

    // 하단 구분선
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #f0f0f0;");
    divider->setFixedHeight(1);
    root->addWidget(divider);
}

// ============================================================
// 서버 데이터 수신 후 UI 채우기
// ============================================================
void StoreItemWidget::setData(const QString &name,
                               const QString &category,
                               double rating, int reviewCount,
                               double distanceKm,
                               int minTime, int maxTime,
                               int deliveryFee,
                               int minOrderAmount,
                               const QStringList &tags)
{
    // 이름
    m_nameLabel->setText(name);

    // 배달 시간
    QString timeStr = (maxTime > 0)
        ? QString("%1~%2분").arg(minTime).arg(maxTime)
        : QString("%1분").arg(minTime);
    m_timeLabel->setText(timeStr);

    // 별점 · 거리 · 배달비
    bool isFree = (deliveryFee == 0);
    m_metaLabel->setText(
        QString("<span>⭐ %1(%2) · %3 · </span>"
                "<span style='color:%4; font-weight:%5;'>%6</span>")
            .arg(rating, 0, 'f', 1).arg(reviewCount)
            .arg(formatDistance(distanceKm))
            .arg(isFree ? "#1565c0" : "#555555")
            .arg(isFree ? "bold" : "normal")
            .arg(formatDeliveryFee(deliveryFee))
    );

    // 최소주문
    m_minOrderLabel->setText("최소주문 " + formatWon(minOrderAmount));

    // placeholder 색상 (이미지 오기 전)
    QColor c = placeholderColor(category);
    m_mainImgLabel->setStyleSheet(
        QString("background-color: %1; font-size:40px;").arg(c.name()));
    m_subImg1Label->setStyleSheet(
        QString("background-color: %1;").arg(c.darker(108).name()));
    m_subImg2Label->setStyleSheet(
        QString("background-color: %1;").arg(c.darker(115).name()));

    // 태그 뱃지 — 기존 거 지우고 새로 추가
    QLayout *tagLayout = m_tagContainer->layout();
    QLayoutItem *child;
    while ((child = tagLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    for (const QString &tag : tags) {
        QLabel *badge = new QLabel(tag);
        badge->setStyleSheet(
            "font-size:11px; color:#1565c0;"
            "background-color:#e8f0ff;"
            "border-radius:4px; padding:2px 7px;");
        tagLayout->addWidget(badge);
    }
    static_cast<QHBoxLayout*>(tagLayout)->addStretch();
}

// ── 이미지 교체 ──
void StoreItemWidget::setMainImage(const QPixmap &pixmap)
{
    m_mainImgLabel->setPixmap(
        pixmap.scaled(m_mainImgLabel->size(),
                      Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}
void StoreItemWidget::setSubImage1(const QPixmap &pixmap)
{
    m_subImg1Label->setPixmap(
        pixmap.scaled(m_subImg1Label->size(),
                      Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}
void StoreItemWidget::setSubImage2(const QPixmap &pixmap)
{
    m_subImg2Label->setPixmap(
        pixmap.scaled(m_subImg2Label->size(),
                      Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

// ── 포맷 헬퍼 ──
QString StoreItemWidget::formatWon(int amount)
{
    return QLocale(QLocale::Korean).toString(amount) + "원";
}
QString StoreItemWidget::formatDeliveryFee(int fee)
{
    return (fee == 0) ? "무료배달"
        : "배달비 " + QLocale(QLocale::Korean).toString(fee) + "원";
}
QString StoreItemWidget::formatDistance(double km)
{
    return (km < 1.0)
        ? QString("%1m").arg(static_cast<int>(km * 1000))
        : QString("%1km").arg(km, 0, 'f', 1);
}
QColor StoreItemWidget::placeholderColor(const QString &category)
{
    if (category == "중식")     return QColor("#fde8d8");
    if (category == "일식")     return QColor("#d8eafd");
    if (category == "치킨")     return QColor("#fdf5d8");
    if (category == "한식")     return QColor("#d8fde4");
    if (category == "양식")     return QColor("#fdd8d8");
    if (category == "카페")     return QColor("#ede8fd");
    if (category == "베이커리") return QColor("#fdf0d8");
    return QColor("#eeeeee");
}
