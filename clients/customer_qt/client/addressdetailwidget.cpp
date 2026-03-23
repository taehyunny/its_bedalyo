#include "addressdetailwidget.h"
#include "ui_addressdetailwidget.h"
#include <QDebug>

AddressDetailWidget::AddressDetailWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddressDetailWidget)
{
    ui->setupUi(this);

    // 라벨 버튼 그룹처럼 동작 (하나만 선택)
    connect(ui->btnLabelHome, &QPushButton::clicked,
            this, &AddressDetailWidget::on_btnLabelHome_clicked);
    connect(ui->btnLabelWork, &QPushButton::clicked,
            this, &AddressDetailWidget::on_btnLabelWork_clicked);
    connect(ui->btnLabelEtc, &QPushButton::clicked,
            this, &AddressDetailWidget::on_btnLabelEtc_clicked);
}

AddressDetailWidget::~AddressDetailWidget() { delete ui; }

// ============================================================
// 새 주소 설정 모드
// ============================================================
void AddressDetailWidget::loadNewAddress(const QString &roadAddr)
{
    m_item = AddressItem();
    m_item.address = roadAddr;
    m_item.label   = "기타";

    ui->labelAddress->setText(roadAddr.isEmpty() ? "주소를 선택해주세요" : roadAddr);
    ui->detailEdit->clear();
    ui->guideEdit->clear();
    updateLabelButtons("기타");

    qDebug() << "[AddressDetailWidget] 새 주소 설정:" << roadAddr;
}

// ============================================================
// 기존 주소 수정 모드
// ============================================================
void AddressDetailWidget::loadEditAddress(const AddressItem &item)
{
    m_item = item;

    ui->labelAddress->setText(item.address);
    ui->detailEdit->setText(item.detail);
    ui->guideEdit->setText(item.guide);
    updateLabelButtons(item.label);

    qDebug() << "[AddressDetailWidget] 주소 수정 모드:" << item.address;
}

// ============================================================
// 라벨 버튼 상태 업데이트
// ============================================================
void AddressDetailWidget::updateLabelButtons(const QString &selected)
{
    ui->btnLabelHome->setChecked(selected == "집");
    ui->btnLabelWork->setChecked(selected == "회사");
    ui->btnLabelEtc->setChecked(selected == "기타");
}

// ============================================================
// 버튼 슬롯
// ============================================================
void AddressDetailWidget::on_btnBack_clicked() { emit backRequested(); }

void AddressDetailWidget::on_btnLabelHome_clicked()
{
    m_item.label = "집";
    updateLabelButtons("집");
}

void AddressDetailWidget::on_btnLabelWork_clicked()
{
    m_item.label = "회사";
    updateLabelButtons("회사");
}

void AddressDetailWidget::on_btnLabelEtc_clicked()
{
    m_item.label = "기타";
    updateLabelButtons("기타");
}

// ============================================================
// 완료 버튼 → AddressWidget으로 결과 전달
// ============================================================
void AddressDetailWidget::on_btnComplete_clicked()
{
    m_item.detail = ui->detailEdit->text().trimmed();
    m_item.guide  = ui->guideEdit->text().trimmed();

    qDebug() << "[AddressDetailWidget] 완료:"
             << m_item.address << m_item.detail << m_item.label;

    emit completed(m_item);
}
