#-------------------------------------------------
#
# Project created by QtCreator 2017-05-16T15:24:06
#
#-------------------------------------------------

QT       += core gui websockets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BajenaghhaClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += OPUS
SOURCES += main.cpp\
        mainwindow.cpp \
    client.cpp \
    bopus.cpp

HEADERS  += mainwindow.h \
    client.h \
    bopus.h

FORMS    += mainwindow.ui


LIBS += -L$$PWD/../opus-android/ -lopus

INCLUDEPATH += $$PWD/../opus-android/include
DEPENDPATH += $$PWD/../opus-android/include

PRE_TARGETDEPS += $$PWD/../opus-android/libopus.a

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../android-openssl-master/prebuilt/armeabi-v7a/libcrypto.so \
        $$PWD/../android-openssl-master/prebuilt/armeabi-v7a/libssl.so
}
