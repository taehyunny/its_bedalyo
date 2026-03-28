#include "wowmembershipdialog.h"
#include "ui_wowmembershipdialog.h"
#include "NetworkManager.h"
#include "UserSession.h"
#include "BaseDTO.h"
#include "Global_protocol.h"
#include <QMessageBox>
#include <QDebug>

// 1. 생성자에서 신호 연결 (NetworkManager를 인자로 받도록 수정하거나 전역 매니저 사용)
WowMembershipDialog::WowMembershipDialog(NetworkManager *network, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WowMembershipDialog),
    m_network(network) // ⚠️ 헤더에 NetworkManager* m_network; 선언 필요
{
    ui->setupUi(this);
    setWindowTitle("와우 멤버십");

    // ✅ 서버 응답 시그널이 오면 내 handleResponse 함수를 실행해라!
    connect(m_network, &NetworkManager::onGradeUpdateResponse, 
            this, &WowMembershipDialog::handleResponse);

    // 🚀 창이 열릴 때 내 아이디와 등급을 바로 표시!
    updateUI();
}

// 🚀 내 정보와 등급 상태를 UI에 그려주는 함수 구현
void WowMembershipDialog::updateUI()
{
    const UserSession &session = UserSession::instance();
    
    // 아이디 표시 (예: asdf님)
    ui->lblUserName->setText(session.userId + "님"); // ✅ lblName -> lblUserName 으로 변경!

    // 등급 표시
    if (session.customerGrade == "와우" || session.customerGrade == "wow") {
        ui->lblStatus->setText("등급: 와우");
        // 와우 회원이면 파란색 굵은 글씨로 강조
        ui->lblStatus->setStyleSheet("font-size: 14px; font-weight: bold; color: #007BFF;"); 
    } else {
        ui->lblStatus->setText("등급: 일반\n지금 가입하고 혜택을 받으세요!");
        // 일반 회원이면 기본 회색 글씨
        ui->lblStatus->setStyleSheet("font-size: 14px; color: #555555;"); 
    }
}

// 2. ✅ 실제 알림창을 띄우는 함수 추가
void WowMembershipDialog::handleResponse(int status, QString message)
{
    qDebug() << "[UI] 서버 응답 처리 중... 상태:" << status;

    if (status == 200 || status == 0) {
        UserSession &session = UserSession::instance();

        if (session.customerGrade == "와우" || session.customerGrade == "wow") {
            session.customerGrade = "일반"; 
            QMessageBox::information(this, "해지 완료", "멤버십이 정상적으로 해지되었습니다.");
        } else {
            session.customerGrade = "와우"; 
            QMessageBox::information(this, "가입 완료", "멤버십 가입이 완료되었습니다!");
        }

        // 🚀 DB 변경 후 바로 화면을 새로고침!
        updateUI(); 

        // ❌ this->accept(); <-- 이 줄을 삭제해야 창이 닫히지 않고 변경된 모습을 바로 볼 수 있습니다!
        
    } else {
        QMessageBox::warning(this, "오류", "요청 실패: " + message);
    }
}

WowMembershipDialog::~WowMembershipDialog()
{
    delete ui;
}

// 🚀 데이터를 동적으로 설정하는 함수 구현 (하드코딩 제거)
// void WowMembershipDialog::setUserInfo(const WowUserData &data)
// {
//     ui->lblUserName->setText(data.userName + "님");
//     ui->lblStatus->setText(data.membershipStatus);
    
//     // ui->lblProfileIcon->setPixmap(QPixmap(data.profilePicPath)); // 필요 시 이미지 설정
// }

// 🚀 버튼 클릭 슬롯 구현
void WowMembershipDialog::on_btnBack_clicked()
{
    this->close();
}

void WowMembershipDialog::on_btnApply_clicked()
{
    const UserSession &session = UserSession::instance();

    if (session.customerGrade == "와우" || session.customerGrade == "wow") {
        qDebug() << " [CHECK] 가입 거절: 이미 와우 등급임 (userId:" << session.userId << ")";
        QMessageBox::warning(this, "알림", "이미 와우 멤버십 회원입니다.");
        return;
    }

    // 서버 전송 준비 로그
    qDebug() << " [REQ_SEND] 등급 변경 요청 전송 시작";
    qDebug() << "  - Protocol ID:" << static_cast<int>(CmdID::REQ_GRADE_UPDATE);
    qDebug() << "  - Target Grade: 'wow', Action: 1 (신규가입)";

    ReqGradeUpdateDTO req;
    req.userId = session.userId.toStdString();
    req.grade = "와우";
    req.action = 1;

    // ❌ 기존: // m_network->sendPacket(CmdID::REQ_GRADE_UPDATE, req);
    // ✅ 수정: 주석(//)을 제거하여 실제 서버로 전송하게 합니다.
    m_network->sendGradeUpdate(req);
    
    qDebug() << " [REQ_DONE] 서버로 실제 패킷 송신 완료!";
    // QMessageBox::information(this, "멤버십 신청", "서버에 가입 요청을 보냈습니다.");
}

void WowMembershipDialog::on_btnTerminate_clicked()
{
    const UserSession &session = UserSession::instance();

    if (session.customerGrade != "와우" && session.customerGrade != "wow") {
        qDebug() << " [CHECK] 해지 거절: 멤버십 미가입 상태 (userId:" << session.userId << ")";
        QMessageBox::critical(this, "오류", "해지 가능한 멤버십이 없습니다.");
        return;
    }

    qDebug() << " [REQ_SEND] 등급 해지 요청 전송 시작";
    qDebug() << "  - Protocol ID:" << static_cast<int>(CmdID::REQ_GRADE_UPDATE);
    qDebug() << "  - Target Grade: '일반', Action: 0 (해지)";

    ReqGradeUpdateDTO req;
    req.userId = session.userId.toStdString();
    req.grade = "일반";
    req.action = 0;

    // ✅ 여기서도 주석을 제거하여 실제 패킷을 보냅니다.
    m_network->sendGradeUpdate(req); 

    qDebug() << " [REQ_DONE] 서버로 해지 패킷 송신 완료!";
}