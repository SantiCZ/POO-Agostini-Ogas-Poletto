#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QFileInfo>

#include "mainwidget.h"
#include "logindialog.h"
#include "datamanager.h"
#include "admindb.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Configuración de fecha y idioma
    QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Argentina));
    QTranslator translator;
    if (translator.load("qt_es", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&translator);

    QFont font("Segoe UI", 10);
    app.setFont(font);

    app.setApplicationName("AlcancIA");
    app.setApplicationVersion("1.0.0");

    // ── 1. Conexión SQLite universal ────────────────────────────────
    adminDB dbProyecto;
    // Usamos QFileInfo(__FILE__) para que funcione en cualquier compu
    QString rutaProyecto = QFileInfo(__FILE__).absolutePath();
    QString rutaDB = rutaProyecto + "/tasty_alcancia.db";

    if (!dbProyecto.conectar(rutaDB)) {
        qDebug() << "Error fatal: No se pudo conectar a SQLite en" << rutaDB;
        return 1;
    }

    // ── 2. Login ────────────────────────────────────────────────────
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0; // Si el usuario cancela el login, salimos
    }

    // ── 3. Sincronización Post-Login ────────────────────────────────
    // Aquí es donde "disparamos" la magia: bajamos datos del VPS y guardamos en SQLite
    // Nota: Deberías obtener el ID real del usuario desde login.getId()
    // Por ahora usamos 1 como ID de ejemplo.
    int id_usuario = 1;
    DataManager::instance().sincronizarDesdeServidor(id_usuario);

    // ── 4. Ventana principal ────────────────────────────────────────
    MainWidget window;
    window.setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
    window.show();

    return app.exec();
}