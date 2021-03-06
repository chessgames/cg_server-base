QT += core websockets sql
QT -= gui

CONFIG += c++11

TARGET = cg_server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    cg_database.cpp \
    cg_server.cpp \
    cg_player.cpp \
    cg_lobby.cpp \
    cg_lobbymanager.cpp \
    cg_usergraph.cpp \
    cg_game.cpp \
    cg_gamemanager.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_SQLITE
#uncomment two lines below to ENABLE testing with QtTest
#DEFINES += CG_TEST_ENABLED
#QT += testlib

# To disable debug output uncomment line below
#DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    cg_database.h \
    cg_server.h \
    cg_global.h \
    cg_player.h \
    cg_lobby.h \
    cg_lobbymanager.h \
    cg_usergraph.h \
    cg_game.h \
    cg_gamemanager.h

unix:{
#LIBS += -lmysqlclient
}

#INCLUDEPATH +=/usr/local/mysql-5.7.18-macos10.12-x86_64/include
mac:{
LIBS +=-L/usr/local/mysql-5.7.18-macos10.12-x86_64/lib -lmysqlclient
}
