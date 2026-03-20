QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
# 공용 헤더 경로 설정 (환경변수 방식)
# 각자 PC에서 ITS_COMMON 환경변수를 common 폴더 경로로 설정하세요.
# 설정 방법은 README.md 참고
# ============================================================
COMMON_PATH = $$(ITS_COMMON)

isEmpty(COMMON_PATH) {
    error("[ 빌드 오류 ] 환경변수 ITS_COMMON 이 설정되지 않았습니다. README.md 를 확인하세요.")
}

INCLUDEPATH += $$COMMON_PATH/include
INCLUDEPATH += $$COMMON_PATH/dto
