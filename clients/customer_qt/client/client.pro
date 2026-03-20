QT += widgets network

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

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
    homewidget.h

FORMS += \
    loginwidget.ui \
    mainwindow.ui \
    homewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/common
