#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "UserSession.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

SettingsWidget::SettingsWidget(NetworkManager *network, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
    , m_network(network)
{
    ui->setupUi(this);
}

SettingsWidget::~SettingsWidget() { delete ui; }

void SettingsWidget::on_btnBack_clicked() { emit backRequested(); }

// ============================================================
// 로그아웃
// ============================================================
void SettingsWidget::on_btnLogout_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "로그아웃", "로그아웃 하시겠습니까?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes)
        emit logoutRequested();
}

// ============================================================
// 회원탈퇴
// TODO: REQ_WITHDRAW(1070) 서버 연동 후 활성화
// ============================================================
void SettingsWidget::on_btnWithdraw_clicked()
{
    // 1차 확인
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "회원탈퇴",
        "정말 탈퇴하시겠습니까?\n탈퇴 시 모든 데이터가 삭제되며 복구할 수 없습니다.",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) return;

    // 2차 비밀번호 확인
    bool ok;
    QString password = QInputDialog::getText(
        this, "비밀번호 확인",
        "비밀번호를 입력해주세요.",
        QLineEdit::Password, "", &ok
    );
    if (!ok || password.isEmpty()) return;

    qDebug() << "[SettingsWidget] 회원탈퇴 요청 - userId:" << UserSession::instance().userId;

    // TODO: REQ_WITHDRAW(1070) 전송
    // WithdrawReqDTO dto;
    // dto.userId   = UserSession::instance().userId.toStdString();
    // dto.password = password.toStdString();
    // m_network->sendWithdraw(dto);

    QMessageBox::information(this, "준비 중", "회원탈퇴 기능은 준비 중입니다.");
}
