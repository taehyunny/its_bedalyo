#include "addressdetailwidget.h"
#include "ui_addressdetailwidget.h"
#include <QMessageBox>
#include <QDebug>

AddressDetailWidget::AddressDetailWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddressDetailWidget)
{
    ui->setupUi(this);

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
    m_item          = AddressItem();
    m_item.address  = roadAddr;
    m_item.label    = "기타";
    m_item.isDefault = false;

    ui->labelAddress->setText(roadAddr.isEmpty() ? "주소를 선택해주세요" : roadAddr);
    ui->detailEdit->clear();
    ui->guideEdit->clear();
    updateLabelButtons("기타");

    // 새 주소 → 삭제 버튼 숨김
    ui->btnDelete->hide();

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

    // 현재 선택된(기본) 주소 → 삭제 버튼 숨김
    // 선택 안 된 주소 → 삭제 버튼 표시
    if (item.isDefault) {
        ui->btnDelete->hide();
    } else {
        ui->btnDelete->show();
        ui->btnDelete->setText("삭제");
    }

    qDebug() << "[AddressDetailWidget] 수정 모드:" << item.address
             << "isDefault:" << item.isDefault;
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
// 완료 버튼
// ============================================================
void AddressDetailWidget::on_btnComplete_clicked()
{
    m_item.detail = ui->detailEdit->text().trimmed();
    m_item.guide  = ui->guideEdit->text().trimmed();

    qDebug() << "[AddressDetailWidget] 완료:"
             << m_item.address << m_item.detail << m_item.label;

    emit completed(m_item);
}

// ============================================================
// 삭제 버튼 (선택 안 된 주소만 표시됨)
// ============================================================
void AddressDetailWidget::on_btnDelete_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "주소 삭제",
        "이 주소를 삭제하시겠습니까?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        qDebug() << "[AddressDetailWidget] 삭제 요청 addressId:" << m_item.addressId;
        emit deleteRequested(m_item.addressId);
    }
}
