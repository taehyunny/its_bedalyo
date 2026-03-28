#include "policywidget.h"
#include "ui_policywidget.h"
#include <QMessageBox>

PolicyWidget::PolicyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PolicyWidget)
{
    ui->setupUi(this);
}

PolicyWidget::~PolicyWidget() { delete ui; }

void PolicyWidget::on_btnBack_clicked() { emit backRequested(); }

void PolicyWidget::on_btnTerms_clicked()
{
    showPolicyText("서비스 이용약관",
        "제1조 (목적)\n"
        "본 약관은 it's 배달요(이하 '회사')가 제공하는 배달 중개 서비스의 이용 조건 및 절차, "
        "회사와 이용자 간의 권리·의무 및 책임 사항을 규정함을 목적으로 합니다.\n\n"
        "제2조 (서비스 이용)\n"
        "이용자는 본 약관에 동의하고 회원가입을 완료한 후 서비스를 이용할 수 있습니다.\n\n"
        "제3조 (개인정보 보호)\n"
        "회사는 관련 법령에 따라 이용자의 개인정보를 보호합니다."
    );
}

void PolicyWidget::on_btnPrivacy_clicked()
{
    showPolicyText("개인정보 처리방침",
        "1. 수집하는 개인정보 항목\n"
        "회사는 서비스 제공을 위해 아이디, 비밀번호, 이름, 전화번호, 주소를 수집합니다.\n\n"
        "2. 개인정보 수집 및 이용 목적\n"
        "회원 관리, 서비스 제공, 배달 서비스 이행을 위해 사용됩니다.\n\n"
        "3. 개인정보 보유 및 이용 기간\n"
        "회원 탈퇴 시까지 보유하며, 관련 법령에 따라 일정 기간 보관될 수 있습니다."
    );
}

void PolicyWidget::on_btnLocation_clicked()
{
    showPolicyText("위치정보 이용약관",
        "제1조 (목적)\n"
        "본 약관은 이용자의 위치정보를 활용하여 주변 배달 가능 매장을 안내하는 서비스 제공을 목적으로 합니다.\n\n"
        "제2조 (위치정보 수집)\n"
        "이용자가 동의한 경우에 한하여 위치정보를 수집하며, 서비스 제공 목적 외에는 사용하지 않습니다."
    );
}

void PolicyWidget::on_btnMarketing_clicked()
{
    showPolicyText("마케팅 정보 수신 동의",
        "마케팅 정보 수신에 동의하시면 이벤트, 할인 쿠폰, 새로운 서비스 안내 등의 정보를 "
        "푸시 알림, 문자메시지, 이메일 등을 통해 받아보실 수 있습니다.\n\n"
        "수신 동의는 언제든지 철회하실 수 있으며, 철회 시 관련 정보 발송이 중단됩니다."
    );
}

void PolicyWidget::showPolicyText(const QString &title, const QString &content)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(content);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}
