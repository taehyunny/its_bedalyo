// menureview.h
#ifndef MENUREVIEW_H
#define MENUREVIEW_H

#include <QWidget>
#include "StoreDetailDTO.h"  // ReviewDTO 사용을 위해 필요
#include "NetworkManager.h"  // 네트워크 요청을 위해 추가

namespace Ui { class menureview; }

class menureview : public QWidget {
    Q_OBJECT
public:
    // [수정] NetworkManager 포인터를 생성자에 추가
    explicit menureview(NetworkManager *network, QWidget *parent = nullptr);
    ~menureview();

    // [추가] menuId를 받아 서버에 리뷰 목록을 요청하는 진입점 함수
    void loadReviews(int menuId);

    void clearReviews();
    void addReviewItem(const ReviewDTO &review);

signals:
    void backRequested();

private slots:
    // [추가] NetworkManager로부터 리뷰 데이터를 수신하는 슬롯
    void onReviewsReceived(int menuId, QList<ReviewDTO> reviews);

private:
    Ui::menureview  *ui;
    NetworkManager  *m_network;
    int              m_menuId = 0;  // 현재 표시 중인 menuId (응답 검증용)
};

#endif // MENUREVIEW_H