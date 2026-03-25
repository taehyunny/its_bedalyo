QT += widgets network

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    cartbarwidget.cpp \
    form.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    NetworkManager.cpp \
    homewidget.cpp \
    menucategori.cpp \
    menuoption.cpp \
    ordercompletewidget.cpp \
    readylist.cpp \
    searchwidget.cpp \
    searchresultwidget.cpp \
    orderhistorywidget.cpp \
    mypagewidget.cpp \
    storedetailwidget.cpp \
    policywidget.cpp \
    settingswidget.cpp \
    addresswidget.cpp \
    addressdetailwidget.cpp \
    cartwidget.cpp


HEADERS += \
    cartbarwidget.h \
    form.h \
    loginwidget.h \
    mainwindow.h \
    NetworkManager.h \
    homewidget.h \
    menucategori.h \
    menuoption.h \
    ordercompletewidget.h \
    readylist.h \
    searchwidget.h \
    searchresultwidget.h \
    orderhistorywidget.h \
    mypagewidget.h \
    UserSession.h \
    storedetailwidget.h \
    storeutils.h \
    config.h \
    cartsession.h \
    policywidget.h \
    settingswidget.h \
    addresswidget.h \
    addressdetailwidget.h \
    cartwidget.h \


FORMS += \
    cartbarwidget.ui \
    form.ui \
    loginwidget.ui \
    mainwindow.ui \
    homewidget.ui \
    menucategori.ui \
    menuoption.ui \
    ordercompletewidget.ui \
    readylist.ui \
    searchwidget.ui \
    searchresultwidget.ui \
    orderhistorywidget.ui \
    mypagewidget.ui \
    storedetailwidget.ui \
    policywidget.ui \
    settingswidget.ui \
    addresswidget.ui \
    addressdetailwidget.ui \
    cartwidget.ui \



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../../../common \
               $$PWD/../../../common/include \
               $$PWD/../../../common/dto \
               $$PWD/../../../common/util
