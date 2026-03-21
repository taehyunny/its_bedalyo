QT += widgets network

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    NetworkManager.cpp \
    homewidget.cpp \
    menucategori.cpp \
    SearchWidget.cpp \
    SearchResultWidget.cpp \
    OrderHistoryWidget.cpp \
    MyPageWidget.cpp

HEADERS += \
    loginwidget.h \
    mainwindow.h \
    NetworkManager.h \
    homewidget.h \
    menucategori.h \
    SearchWidget.h \
    SearchResultWidget.h \
    OrderHistoryWidget.h \
    MyPageWidget.h \
    UserSession.h

FORMS += \
    loginwidget.ui \
    mainwindow.ui \
    homewidget.ui \
    menucategori.ui \
    searchwidget.ui \
    searchresultwidget.ui \
    orderhistorywidget.ui \
    mypagewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../../../common \
               $$PWD/../../../common/include \
               $$PWD/../../../common/dto \
               $$PWD/../../../common/util
