#include "MyPageWidget.h"
#include "ui_mypagewidget.h"
#include "UserSession.h"
#include <QMessageBox>
#include <QDebug>

// ============================================================
// 생성자
// ============================================================
MyPageWidget::MyPageWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyPageWidget)
    , m_network(network)
{
    ui->setupUi(this);

    // ── 메뉴 버튼 연결 ──
    connect(ui->btnDetail,   &QPushButton::clicked, this, &MyPageWidget::on_btnDetail_clicked);
    connect(ui->btnAddress,  &QPushButton::clicked, this, &MyPageWidget::on_btnAddress_clicked);
    connect(ui->btnFavorite, &QPushButton::clicked, this, &MyPageWidget::on_btnFavorite_clicked);
    connect(ui->btnCoupon,   &QPushButton::clicked, this, &MyPageWidget::on_btnCoupon_clicked);
    connect(ui->btnFaq,      &QPushButton::clicked, this, &MyPageWidget::on_btnFaq_clicked);
    connect(ui->btnPolicy,   &QPushButton::clicked, this, &MyPageWidget::on_btnPolicy_clicked);
    connect(ui->btnSettings, &QPushButton::clicked, this, &MyPageWidget::on_btnSettings_clicked);
    connect(ui->btnLogout,   &QPushButton::clicked, this, &MyPageWidget::on_btnLogout_clicked);

    // ── 내비바 연결 ──
    connect(ui->navHome,     &QPushButton::clicked, this, &MyPageWidget::on_navHome_clicked);
    connect(ui->navSearch,   &QPushButton::clicked, this, &MyPageWidget::on_navSearch_clicked);
    connect(ui->navFavorite, &QPushButton::clicked, this, &MyPageWidget::on_navFavorite_clicked);
    connect(ui->navOrder,    &QPushButton::clicked, this, &MyPageWidget::on_navOrder_clicked);
    connect(ui->navMy,       &QPushButton::clicked, this, &MyPageWidget::on_navMy_clicked);

    // ── TODO: 서버 응답 연결 ──
    // connect(m_network, &NetworkManager::onMyInfoReceived,
    //         this, &MyPageWidget::onMyInfoReceived);
}

MyPageWidget::~MyPageWidget() { delete ui; }

// ============================================================
// 화면 진입 시 호출
// ============================================================
void MyPageWidget::loadData()
{
    // UserSession 정보로 프로필 UI 갱신
    updateProfileUI();

    qDebug() << "[MyPageWidget] loadData() - userId:" << UserSession::instance().userId;

    // TODO: REQ_MY_INFO(2100) 전송 — 리뷰수/도움됐어요/즐겨찾기수
    // MyInfoReqDTO dto;
    // dto.userId = UserSession::instance().userId.toStdString();
    // m_network->sendMyInfo(dto);
}

// ============================================================
// UserSession → 프로필 UI 갱신
// ============================================================
void MyPageWidget::updateProfileUI()
{
    const UserSession &session = UserSession::instance();

    ui->labelName->setText(session.userName.isEmpty() ? "-" : session.userName);
    ui->labelPhone->setText(
        session.phoneNumber.isEmpty() ? "-" : maskPhoneNumber(session.phoneNumber)
    );

    // 통계는 서버 응답 전까지 0으로 표시
    ui->labelReviewCount->setText("0");
    ui->labelHelpCount->setText("0");
    ui->labelFavoriteCount->setText("0");
}

// ============================================================
// 통계 카운트 업데이트 (서버 응답 수신 후 MainWindow 또는 슬롯에서 호출)
// ============================================================
void MyPageWidget::setStats(int reviewCount, int helpCount, int favoriteCount)
{
    ui->labelReviewCount->setText(QString::number(reviewCount));
    ui->labelHelpCount->setText(QString::number(helpCount));
    ui->labelFavoriteCount->setText(QString::number(favoriteCount));
}

// ============================================================
// 전화번호 마스킹
// 010-1234-5678 → 010-****-5678
// ============================================================
QString MyPageWidget::maskPhoneNumber(const QString &phone)
{
    // 하이픈 포함 형식: 010-XXXX-XXXX
    QStringList parts = phone.split("-");
    if (parts.size() == 3) {
        return parts[0] + "-****-" + parts[2];
    }
    // 하이픈 없는 형식: 01012345678 → 010-****-5678
    if (phone.length() == 11) {
        return phone.left(3) + "-****-" + phone.right(4);
    }
    return phone; // 형식 불명 → 그대로 표시
}

// ============================================================
// 메뉴 버튼 슬롯
// ============================================================
void MyPageWidget::on_btnDetail_clicked()
{
    // TODO: 프로필 상세 화면으로 전환
    QMessageBox::information(this, "준비 중", "프로필 상세 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnAddress_clicked()
{
    // TODO: 주소 관리 화면으로 전환
    QMessageBox::information(this, "준비 중", "주소 관리 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnFavorite_clicked()
{
    emit favoriteRequested();
}

void MyPageWidget::on_btnCoupon_clicked()
{
    // TODO: 쿠폰·이용권 화면으로 전환
    QMessageBox::information(this, "준비 중", "쿠폰·이용권 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnFaq_clicked()
{
    // TODO: FAQ 화면으로 전환
    QMessageBox::information(this, "준비 중", "자주 묻는 질문 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnPolicy_clicked()
{
    // TODO: 약관 및 정책 화면으로 전환
    QMessageBox::information(this, "준비 중", "약관 및 정책 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnSettings_clicked()
{
    // TODO: 설정 화면으로 전환
    QMessageBox::information(this, "준비 중", "설정 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnLogout_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "로그아웃", "로그아웃 하시겠습니까?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes)
        emit logoutRequested();
}

// ============================================================
// 내비바 슬롯
// ============================================================
void MyPageWidget::on_navHome_clicked()     { emit homeRequested(); }
void MyPageWidget::on_navSearch_clicked()   { emit searchRequested(); }
void MyPageWidget::on_navFavorite_clicked() { emit favoriteRequested(); }
void MyPageWidget::on_navOrder_clicked()    { emit orderListRequested(); }
void MyPageWidget::on_navMy_clicked()       {} // 현재 화면 — 아무 동작 없음
