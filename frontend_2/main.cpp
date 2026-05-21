#include "mainwidget.h"
#include "logindialog.h"
#include "datamanager.h"
#include "admindb.h"

#include <QApplication>
#include <QFont>
#include <QMessageBox>
#include <QDebug>     // Permite mostrar mensajes en consola
#include <QSqlQuery> // Permite ejecutar consultas SQL
#include <QSqlError> // Permite leer errores SQL

#include <QFile>
#include <QIODevice>
#include <QDir>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);

    app.setApplicationName("AlcancIA");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AlcancIA Team");

    // Creamos el objeto que maneja la base
    adminDB dbProyecto;

    // Intentamos conectar la base SQLite
    if (dbProyecto.conectar("C:/sqlite/tasty_alcancia.db"))
    {
        qDebug() << "Base conectada correctamente";

        QFile archivoJson("sync_test.json");

        qDebug() << "Buscando JSON en:" << QDir::currentPath();

        if (archivoJson.open(QIODevice::ReadOnly))
        {
            QByteArray datosJson = archivoJson.readAll();
            archivoJson.close();

            dbProyecto.sincronizarDesdeJson(datosJson);
        }
        else
        {
            qDebug() << "No se pudo abrir sync_test.json";
        }
    }
    else
    {
        qDebug() << "No se pudo conectar la base";
        return 1;
    }

    // Cargar datos existentes
    DataManager::instance().loadFromFile();
    // Crear usuario demo si no hay ninguno
    DataManager::instance().migrateUsers();

    // MOSTRAR USUARIOS
    //---------------------------------------------------------

    // Creamos una consulta usando la conexión
    QSqlQuery queryTablas(dbProyecto.getDB());

    qDebug() << "\n=== TABLAS EN LA BASE ===";

    // Ejecutamos la consulta
    if(queryTablas.exec(
            "SELECT name "
            "FROM sqlite_master "
            "WHERE type='table';"
            ))
    {
        qDebug() << "";
        qDebug() << "===== TABLAS Y CAMPOS =====";


        // Recorremos cada tabla encontrada
        while(queryTablas.next())
        {
            // Guardamos el nombre de la tabla
            QString nombreTabla =
                queryTablas.value(0).toString();

            qDebug() << "";
            qDebug() << "Tabla:" << nombreTabla;


            // ============================
            // CONSULTAR CAMPOS
            // ============================

            QSqlQuery queryCampos(dbProyecto.getDB());


            // PRAGMA table_info(tabla)
            // devuelve información de columnas

            QString consultaCampos =
                "PRAGMA table_info(" +
                nombreTabla + ");";


            if(queryCampos.exec(consultaCampos))
            {
                while(queryCampos.next())
                {
                    // Columna 1 = nombre del campo
                    QString nombreCampo =
                        queryCampos.value(1).toString();

                    // Columna 2 = tipo del campo
                    QString tipoCampo =
                        queryCampos.value(2).toString();

                    qDebug()
                        << "   Campo:"
                        << nombreCampo
                        << "| Tipo:"
                        << tipoCampo;
                }
            }
            else
            {
                qDebug()
                << "Error leyendo campos:"
                << queryCampos.lastError().text();
            }
        }
    }
    else
    {
        qDebug()
        << "Error leyendo tablas:"
        << queryTablas.lastError().text();
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    // Si el usuario se acaba de registrar, mostrar mensaje de bienvenida
    if (login.wasRegistered()) {
        QMessageBox msg;
        msg.setWindowTitle("Bienvenido a AlcancIA");
        msg.setText(QString("Cuenta creada exitosamente.\n\nHola, %1! Ya podés empezar.").arg(login.getUsername()));
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

    MainWidget window;
    window.setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
    window.show();

    return app.exec();
}