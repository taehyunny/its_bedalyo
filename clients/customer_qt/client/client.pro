QT += widgets network

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    NetworkManager.cpp \
    homewidget.cpp \
    storeitemwidget.cpp

HEADERS += \
    loginwidget.h \
    mainwindow.h \
    NetworkManager.h \
    homewidget.h \
    storeitemwidget.h

FORMS += \
    loginwidget.ui \
    mainwindow.ui \
    homewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ============================================================
# 공용 헤더 경로 (common 폴더를 client 안에 복사해서 사용)
# 새 DTO 필요 시 common/ 폴더에 파일 추가 후 여기 경로 그대로 사용
# ============================================================
INCLUDEPATH += $$PWD/common