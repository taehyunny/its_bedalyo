#pragma once
#include <QMainWindow>
#include "NetworkManager.h"
#include "LoginWidget.h"
#include "HomeWidget.h"
#include "menucategori.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// ============================================================
// MainWindow - 앱 최상위 컨테이너
//
// - QStackedWidget으로 화면 전환 관리
// - NetworkManager를 생성해 각 Widget에 주입
// - 카테고리 목록을 캐싱해 menucategori 진입 시 재사용
//   (서버에서 한 번만 받고 재요청 안 함)
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showLogin();
    void showHome();

private slots:
    void onLoginSuccess();
    void onLogoutRequested();
    void onCategorySelected(int categoryId, const QString &categoryName);
    void onBackToHome();

    // 카테고리 데이터 캐싱 (menucategori 진입 시 재사용)
    void onMainHomeReceived(QList<CategoryInfoQt> categories,
                            QList<TopStoreInfoQt> topStores);

private:
    Ui::MainWindow *ui;
    NetworkManager *m_network;
    LoginWidget    *m_loginWidget;
    HomeWidget     *m_homeWidget;
    menucategori   *m_menuWidget;

    // ── 홈에서 받은 카테고리 목록 캐싱 (menucategori에서 재사용) ──
    QList<CategoryInfoQt> m_cachedCategories;
};
