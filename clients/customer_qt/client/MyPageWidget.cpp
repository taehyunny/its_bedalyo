#include "mypagewidget.h"
#include "ui_mypagewidget.h"
#include "UserSession.h"
#include <QMessageBox>
#include <QDebug>

MyPageWidget::MyPageWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyPageWidget)
    , m_network(network)
{
    ui->setupUi(this);
}

MyPageWidget::~MyPageWidget() { delete ui; }

void MyPageWidget::loadData()
{
    updateProfileUI();
    qDebug() << "[MyPageWidget] loadData() - userId:" << UserSession::instance().userId;

    // TODO: REQ_MY_INFO(2100) 전송
}

void MyPageWidget::updateProfileUI()
{
    const UserSession &session = UserSession::instance();
    ui->labelName->setText(session.userName.isEmpty() ? "-" : session.userName);
    ui->labelPhone->setText(
        session.phoneNumber.isEmpty() ? "-" : maskPhoneNumber(session.phoneNumber)
    );
    ui->labelReviewCount->setText("0");
    ui->labelHelpCount->setText("0");
    ui->labelFavoriteCount->setText("0");
}

void MyPageWidget::setStats(int reviewCount, int helpCount, int favoriteCount)
{
    ui->labelReviewCount->setText(QString::number(reviewCount));
    ui->labelHelpCount->setText(QString::number(helpCount));
    ui->labelFavoriteCount->setText(QString::number(favoriteCount));
}

QString MyPageWidget::maskPhoneNumber(const QString &phone)
{
    QStringList parts = phone.split("-");
    if (parts.size() == 3)
        return parts[0] + "-****-" + parts[2];
    if (phone.length() == 11)
        return phone.left(3) + "-****-" + phone.right(4);
    return phone;
}

void MyPageWidget::on_btnDetail_clicked()
{
    QMessageBox::information(this, "준비 중", "프로필 상세 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnAddress_clicked()
{
    emit addressRequested();
}

void MyPageWidget::on_btnFavorite_clicked()  { emit favoriteRequested(); }
void MyPageWidget::on_btnPolicy_clicked()    { emit policyRequested(); }
void MyPageWidget::on_btnSettings_clicked()  { emit settingsRequested(); }

void MyPageWidget::on_btnWow_clicked()
{
    QMessageBox::information(this, "준비 중", "와우 멤버십 화면은 준비 중입니다.");
}

void MyPageWidget::on_btnFaq_clicked()
{
    QMessageBox::information(this, "준비 중", "자주 묻는 질문 화면은 준비 중입니다.");
}

void MyPageWidget::on_navHome_clicked()     { emit homeRequested(); }
void MyPageWidget::on_navSearch_clicked()   { emit searchRequested(); }
void MyPageWidget::on_navFavorite_clicked() { emit favoriteRequested(); }
void MyPageWidget::on_navOrder_clicked()    { emit orderListRequested(); }
void MyPageWidget::on_navMy_clicked()       {}
