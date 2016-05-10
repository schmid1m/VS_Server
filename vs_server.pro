TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    VS_LAB/PacketLib.c \
    VS_LAB/serverAPI.c \
    plreglib.c \
    timeoutlib.c

HEADERS += \
    VS_LAB/commonAPI.h \
    VS_LAB/internalMacros.h \
    VS_LAB/Macros.h \
    VS_LAB/PacketLib.h \
    VS_LAB/serverAPI.h \
    plreglib.h \
    timeoutlib.h

