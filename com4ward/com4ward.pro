#-------------------------------------------------
#
# Project created by QtCreator 2018-12-11T16:36:09
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = com4ward
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES -= UNICODE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += \
    $$PWD/com2tcp \
    $$PWD/../3rdparty/include \
    $$PWD/../3rdparty/include/mqtt \
    $$PWD/../3rdparty/include/qextserialport \
    $$PWD/keyhelper \

win32{
    LIBS += \
        -lsetupapi \
        -lAdvapi32 \
        -lodbc32 \
        -lodbccp32 \
        -lws2_32 \

    CONFIG(debug, debug|release) {
        LIBS += \
            -l$$PWD/../3rdparty/libs/paho-mqttpp3-static_d \
            -l$$PWD/../3rdparty/libs/paho-mqtt3c_d \
            -l$$PWD/../3rdparty/libs/paho-mqtt3a_d \
            -l$$PWD/../3rdparty/libs/qextserialportd \
    } else {
        LIBS += \
            -l$$PWD/../3rdparty/libs/paho-mqttpp3-static \
            -l$$PWD/../3rdparty/libs/paho-mqtt3c \
            -l$$PWD/../3rdparty/libs/paho-mqtt3a \
            -l$$PWD/../3rdparty/libs/qextserialport \
    }
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    logger.cpp \
    com2tcpthread.cpp \
    lightwidget.cpp \
    com2tcp/com2tcp.cpp \
    com2tcp/telnet.cpp \
    com2tcp/utils.cxx \
    keyhelper/keytemplate.cpp \
    keyhelper/keyhelper.cpp \
    keyhelper/simplecrypt.cpp \
    hled.cpp \

HEADERS += \
    mainwindow.h \
    logger.h \
    com2tcpthread.h \
    lightwidget.h \
    com2tcp/precomp.h \
    com2tcp/telnet.h \
    com2tcp/utils.h \
    com2tcp/version.h \
    keyhelper/keytemplate.h \
    keyhelper/keyhelper.h \
    keyhelper/simplecrypt.h \
    hled.h \

win32{
    SOURCES += \
        keyhelper/keyhelper_win.cpp
    LIBS += \
         -lAdvapi32 \
         -lIphlpapi \
} osx{
    SOURCES += \
        keyhelper/keyhelper_mac.cpp
}linux{
    SOURCES += \
        keyhelper/keyhelper_linux.cpp
}

FORMS += \
        mainwindow.ui
