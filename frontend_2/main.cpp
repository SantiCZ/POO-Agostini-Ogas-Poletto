#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QIODevice>
#include <QDir>

#include "mainwidget.h"
#include "logindialog.h"
#include "datamanager.h"
#include "admindb.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Configuración de fecha en español (DEBE IR ANTES DE CREAR CUALQUIER QDATE)
    QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Argentina));
    QTranslator translator;
    if (translator.load("qt_es", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&translator);

    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);

    app.setApplicationName("AlcancIA");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AlcancIA Team");

    // ── Conexión SQLite via adminDB ────────────────────────────────
    adminDB dbProyecto;

    if (dbProyecto.conectar("C:/sqlite/tasty_alcancia.db")) {
        qDebug() << "Base conectada correctamente";

        QFile archivoJson("sync_test.json");
        qDebug() << "Buscando JSON en:" << QDir::currentPath();

        if (archivoJson.open(QIODevice::ReadOnly)) {
            QByteArray datosJson = archivoJson.readAll();
            archivoJson.close();
            dbProyecto.sincronizarDesdeJson(datosJson);
        } else {
            qDebug() << "No se pudo abrir sync_test.json";
        }

        // ── Mostrar estructura de la base en consola ───────────────
        QSqlQuery queryTablas(dbProyecto.getDB());
        qDebug() << "\n===== TABLAS Y CAMPOS =====";

        if (queryTablas.exec("SELECT name FROM sqlite_master WHERE type='table';")) {
            while (queryTablas.next()) {
                QString nombreTabla = queryTablas.value(0).toString();
                qDebug() << "\nTabla:" << nombreTabla;

                QSqlQuery queryCampos(dbProyecto.getDB());
                if (queryCampos.exec("PRAGMA table_info(" + nombreTabla + ");")) {
                    while (queryCampos.next()) {
                        qDebug() << "   Campo:"
                                 << queryCampos.value(1).toString()
                                 << "| Tipo:"
                                 << queryCampos.value(2).toString();
                    }
                } else {
                    qDebug() << "Error leyendo campos:" << queryCampos.lastError().text();
                }
            }
        } else {
            qDebug() << "Error leyendo tablas:" << queryTablas.lastError().text();
        }

    } else {
        qDebug() << "No se pudo conectar la base";
        return 1;
    }

    // ── DataManager ────────────────────────────────────────────────
    DataManager::instance().loadFromFile();
    DataManager::instance().migrateUsers();

    // ── Login / Registro ───────────────────────────────────────────
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    if (login.wasRegistered()) {
        QMessageBox msg;
        msg.setWindowTitle("Bienvenido a AlcancIA");
        msg.setText(QString("Cuenta creada exitosamente.\n\nHola, %1! Ya podés empezar.")
                        .arg(login.getUsername()));
        msg.setStyleSheet(R"(
            QMessageBox { background-color: #1A1D27; }
            QMessageBox QLabel { color: #F1F5F9; font-size: 13px; }
            QMessageBox QPushButton {
                background-color: #4ADE80; color: #0F1117;
                border: none; border-radius: 6px;
                padding: 6px 20px; font-weight: 700;
            }
            QMessageBox QPushButton:hover { background-color: #22C55E; }
        )");
        msg.exec();
    }

    // ── Ventana principal ──────────────────────────────────────────
    MainWidget window;
    window.setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
    window.show();

    return app.exec();
}