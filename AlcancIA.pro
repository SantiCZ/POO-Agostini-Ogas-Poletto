QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = AlcancIA
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sidebar.cpp \
    dashboardpage.cpp \
    ticketspage.cpp \
    subscriptionspage.cpp \
    reportspage.cpp \
    uploadticketdialog.cpp \
    addsubscriptiondialog.cpp \
    stylemanager.cpp \
    datamanager.cpp \
    logindialog.cpp

HEADERS += \
    mainwindow.h \
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