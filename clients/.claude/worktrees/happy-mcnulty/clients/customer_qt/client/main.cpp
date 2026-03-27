#include "mainwindow.h"
#include <QApplication>

// ============================================================
// 진입점
// QApplication 생성 → MainWindow 실행 → 이벤트 루프 시작
// ============================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Qt 앱 초기화 (이벤트 루프, 폰트, 스타일 등 설정)
    MainWindow w;               // 최상위 창 생성 (내부에서 서버 연결 시도)
    w.show();                   // 창 표시
    return QCoreApplication::exec(); // 이벤트 루프 시작 (창이 닫힐 때까지 대기)
}
