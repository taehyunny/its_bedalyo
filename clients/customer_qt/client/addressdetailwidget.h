#pragma once
#include <QWidget>
#include "addresswidget.h" // AddressItem 사용

QT_BEGIN_NAMESPACE
namespace Ui { class AddressDetailWidget; }
QT_END_NAMESPACE

// ============================================================
// AddressDetailWidget - 주소 설정 화면
//
// [ 기능 ]
// - 선택된 주소 표시
// - 상세주소 입력
// - 길안내 입력
// - 집 / 회사 / 기타 선택
// - 완료 버튼 → 주소 저장 후 AddressWidget으로 복귀
// ============================================================
class AddressDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddressDetailWidget(QWidget *parent = nullptr);
    ~AddressDetailWidget();

    // 새 주소 설정 모드
    void loadNewAddress(const QString &roadAddr);

    // 기존 주소 수정 모드
    void loadEditAddress(const AddressItem &item);

signals:
    void backRequested();
    void completed(const AddressItem &item); // 완료 시 emit

private slots:
    void on_btnBack_clicked();
    void on_btnComplete_clicked();
    void on_btnLabelHome_clicked();
    void on_btnLabelWork_clicked();
    void on_btnLabelEtc_clicked();

private:
    Ui::AddressDetailWidget *ui;
    AddressItem              m_item; // 현재 편집 중인 주소

    void updateLabelButtons(const QString &selected);
};
