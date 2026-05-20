QT += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = frontend_2
TEMPLATE = app

SOURCES += \
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