QT += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = frontend_2
TEMPLATE = app

SOURCES += \
    admindb.cpp \
    main.cpp \
    mainwidget.cpp \
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


