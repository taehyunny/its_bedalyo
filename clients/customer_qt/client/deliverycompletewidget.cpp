#include "deliverycompletewidget.h"
#include "ui_deliverycompletewidget.h"
#include <QPushButton>

DeliveryCompleteWidget::DeliveryCompleteWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeliveryCompleteWidget)
    , m_network(network) // 메인 윈도우가 넘겨준 통신 관리자 저장!
    , m_rating(0)        // 별점은 0점부터 시작
{
    ui->setupUi(this);

    // 버튼 5개가 눌렸을 때 각각 1~5점을 함수로 전달하도록 연결
    connect(ui->btnStar1, &QPushButton::clicked, this, [this]() { updateStars(1); });
    connect(ui->btnStar2, &QPushButton::clicked, this, [this]() { updateStars(2); });
    connect(ui->btnStar3, &QPushButton::clicked, this, [this]() { updateStars(3); });
    connect(ui->btnStar4, &QPushButton::clicked, this, [this]() { updateStars(4); });
    connect(ui->btnStar5, &QPushButton::clicked, this, [this]() { updateStars(5); });
    //  등록하기 버튼을 누르면 화면 전환 신호를 쏩니다.
    connect(ui->btnRegister, &QPushButton::clicked, this, [this]() {
        emit orderListRequested(); 
    });

}

DeliveryCompleteWidget::~DeliveryCompleteWidget()
{
    delete ui;
}

// 전달받은 점수만큼 별을 색칠하는 함수
void DeliveryCompleteWidget::updateStars(int rating)
{
    m_rating = rating; // 선택된 점수 저장

    // 5개의 버튼을 배열로 묶기
    QPushButton* stars[5] = {
        ui->btnStar1, ui->btnStar2, ui->btnStar3, ui->btnStar4, ui->btnStar5
    };

    // 1번 별부터 5번 별까지 차례대로 검사
    for (int i = 0; i < 5; ++i) {
        if (i < rating) {
            // 꽉 찬 별(★) + 황금색
            stars[i]->setText("★");
            stars[i]->setStyleSheet("border: 1px solid #cccccc; color: #FFD700; font-size: 24px; font-weight: bold; border-radius: 4px; background-color: transparent;");
        } else {
            // 빈 별(☆) + 회색
            stars[i]->setText("☆");
            stars[i]->setStyleSheet("border: 1px solid #cccccc; color: #cccccc; font-size: 24px; font-weight: bold; border-radius: 4px; background-color: transparent;");
        }
    }
}