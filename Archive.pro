QT += core gui widgets

TEMPLATE = app
TARGET = Archive

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Optional: help you catch deprecated API usage when moving to Qt 6
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
