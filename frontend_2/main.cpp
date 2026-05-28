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

    QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Argentina));
    QTranslator translator;
    if (translator.load("qt_es", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&translator);

    QFont font("Segoe UI", 10);
    app.setFont(font);
    app.setApplicationName("AlcancIA");
    app.setApplicationVersion("1.0.0");

    // ── 1. Conexión SQLite ───────────────────────────────────────────
    adminDB dbProyecto;
    QString rutaDB = QFileInfo(__FILE__).absolutePath() + "/tasty_alcancia.db";
    if (!dbProyecto.conectar(rutaDB)) {
        qDebug() << "Error fatal: No se pudo conectar a SQLite en" << rutaDB;
        return 1;
    }

    qDebug() << "SQLite conectado en:" << rutaDB;
    qDebug() << "Ruta absoluta SQLite:" << QFileInfo(rutaDB).absoluteFilePath();
    qDebug() << "Existe SQLite:" << QFileInfo(rutaDB).exists();

    QString ultimoNombre;
    QString ultimoEmail;

    if (dbProyecto.obtenerUltimoUsuario(ultimoNombre, ultimoEmail))
    {
        qDebug() << "Ultimo usuario usado:"
                 << ultimoNombre
                 << ultimoEmail;
    }

    // ── 2. Login ─────────────────────────────────────────────────────
    LoginDialog login;
    if (login.exec() != QDialog::Accepted)
        return 0;

    // ── 3. Sincronización post-login ─────────────────────────────────
    // CORREGIDO: no llamar sincronizarDesdeServidor() acá con ID hardcodeado.
    // El login real via loginRed() ya dispara sincronizarDesdeServidor()
    // automaticamente cuando el VPS responde OK.
    // Si se llama acá con id=1, se bajan datos del usuario 1 sin importar
    // quien hizo login, y luego el sync real los pisa o duplica.

    // ── 4. Ventana principal ─────────────────────────────────────────
    MainWidget window;
    window.setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
    window.show();

    return app.exec();
}