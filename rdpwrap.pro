QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bitmap.cpp \
    cache.cpp \
    iso.cpp \
    licence.cpp \
    main.cpp \
    mainwindow.cpp \
    mcs.cpp \
    orders.cpp \
    rdesktop.cpp \
    rdp.cpp \
    secure.cpp \
    tcp.cpp \
    xkeymap.cpp \
    xwin.cpp

HEADERS += \
    constants.h \
    mainwindow.h \
    orders.h \
    parse.h \
    proto.h \
    rdesktop.h \
    scancodes.h \
    types.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += "./third_party"
LIBS += "C:\\OpenSSL-Win32\\lib\\MinGW\\libcrypto-1_1.a" \
    "C:\\OpenSSL-Win32\\lib\\MinGW\\libssl-1_1.a"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    rdpwrap.pro.user
