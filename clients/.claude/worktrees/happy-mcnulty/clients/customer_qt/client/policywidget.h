#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class PolicyWidget; }
QT_END_NAMESPACE

// ============================================================
// PolicyWidget - 약관 및 정책 화면
// 서버 통신 없음 — 텍스트만 출력
// ============================================================
class PolicyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PolicyWidget(QWidget *parent = nullptr);
    ~PolicyWidget();

signals:
    void backRequested();

private slots:
    void on_btnBack_clicked();
    void on_btnTerms_clicked();
    void on_btnPrivacy_clicked();
    void on_btnLocation_clicked();
    void on_btnMarketing_clicked();

private:
    Ui::PolicyWidget *ui;

    void showPolicyText(const QString &title, const QString &content);
};
