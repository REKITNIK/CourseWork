QT += core gui widgets sql

CONFIG += c++17

TARGET = CourseProject
TEMPLATE = app

DESTDIR = bin

SOURCES += \
    src/main.cpp \
    src/db/DatabaseManager.cpp \
    src/core/CryptoUtils.cpp \
    src/core/CourseManager.cpp \
    src/ui/LoginDialog.cpp \
    src/ui/AdminWindow.cpp \
    src/ui/StudentWindow.cpp

HEADERS += \
    src/core/AppSettings.h \
    src/db/DatabaseManager.h \
    src/models/Structures.h \
    src/core/CryptoUtils.h \
    src/core/CourseManager.h \
    src/ui/LoginDialog.h \
    src/ui/AdminWindow.h \
    src/ui/StudentWindow.h

INCLUDEPATH += src

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libpq
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

RESOURCES += \
    resources.qrc
