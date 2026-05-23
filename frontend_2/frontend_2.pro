QT       += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -Wno-error=implicit-function-declaration

TARGET = frontend_2
TEMPLATE = app

SOURCES += \
    addsubscriptiondialog.cpp \
    admindb.cpp \
    dashboardpage.cpp \
    datamanager.cpp \
    logindialog.cpp \
    main.cpp \
    mainwidget.cpp \
    reportspage.cpp \
    sidebar.cpp \
    stylemanager.cpp \
    subscriptionspage.cpp \
    ticketspage.cpp \
    uploadticketdialog.cpp \


HEADERS += \
    admindb.h \
    mainwidget.h \
    sidebar.h \
    dashboardpage.h \
    ticketspage.h \
    subscriptionspage.h \
    reportspage.h \
    uploadticketdialog.h \
    addsubscriptiondialog.h \
    stylemanager.h \
    models.h \
    datamanager.h \
    logindialog.h