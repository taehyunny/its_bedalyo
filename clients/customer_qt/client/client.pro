QT += widgets network

CONFIG += c++17

SOURCES += \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    NetworkManager.cpp \
    homewidget.cpp

HEADERS += \
    loginwidget.h \
    mainwindow.h \
    NetworkManager.h \
    homewidget.h \
    UserSession.h

FORMS += \
    loginwidget.ui \
    mainwindow.ui \
    homewidget.ui

INCLUDEPATH += $$PWD/common

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
