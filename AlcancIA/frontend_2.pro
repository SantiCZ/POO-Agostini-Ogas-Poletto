# Modulos Qt utilizados por la aplicacion:
# - widgets/gui/core para la interfaz
# - network para comunicacion con el VPS
# - sql para persistencia local SQLite
QT       += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# El proyecto usa caracteristicas modernas de C++.
CONFIG += c++17
QMAKE_CXXFLAGS += -Wno-error=implicit-function-declaration

# Configuracion general de la aplicacion ejecutable.
TARGET = frontend_2
TEMPLATE = app

# Archivos de implementacion.
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

# Headers con declaraciones de clases, modelos y componentes UI.
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
