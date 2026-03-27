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
}

// 2. ✅ 실제 알림창을 띄우는 함수 추가
void WowMembershipDialog::handleResponse(int status, QString message)
{
    qDebug() << "[UI] 서버 응답 처리 중... 상태:" << status;

    // 1. 서버가 보내는 성공 코드인 200을 조건에 추가합니다.
    if (status == 200 || status == 0) { 
        
        // 2. 🚀 핵심: DB가 바뀌었으니, 내 프로그램(로컬)의 등급 정보도 바꿔줍니다.
        UserSession &session = UserSession::instance();
        
        if (session.customerGrade == "와우" || session.customerGrade == "wow") {
            session.customerGrade = "일반";
        } else {
            session.customerGrade = "와우";
        }

        // 서버에서 보내준 메시지를 그대로 띄워줍니다 ("이제 배달비가 0원입니다" 등)
        QMessageBox::information(this, "성공", message); 
        this->accept(); // 확인 누르면 창 닫기
        
    } else {
        QMessageBox::warning(this, "오류", "변경 실패: " + message);
    }
}

WowMembershipDialog::~WowMembershipDialog()
{
    delete ui;
}

// 🚀 데이터를 동적으로 설정하는 함수 구현 (하드코딩 제거)
void WowMembershipDialog::setUserInfo(const WowUserData &data)
{
    ui->lblUserName->setText(data.userName + "님");
    ui->lblStatus->setText(data.membershipStatus);
    
    // ui->lblProfileIcon->setPixmap(QPixmap(data.profilePicPath)); // 필요 시 이미지 설정
}

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