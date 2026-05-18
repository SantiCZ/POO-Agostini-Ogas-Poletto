#include "admindbproyecto.h"
#include <QApplication>
#include <QDebug> // Permite mostrar mensajes en consola
#include <QSqlQuery> // Permite ejecutar consultas SQL
#include <QSqlError> // Permite leer errores SQL

int main(int argc, char *argv[])
{
    // Creamos la aplicación Qt
    QApplication a(argc, argv);

    // Creamos el objeto que maneja la base
    adminDBproyecto dbProyecto;

    // Intentamos conectar la base SQLite
    if (dbProyecto.conectar("C:/sqlite/tasty_alcancia.db"))
    {
        qDebug() << "Base conectada correctamente";
    }
    else
    {
        qDebug() << "No se pudo conectar la base";
        return 1;
    }

    // MOSTRAR USUARIOS
    //---------------------------------------------------------

    // Creamos una consulta usando la conexión
    QSqlQuery queryTablas(dbProyecto.getDB());


    // Preparamos la consulta SQL
    /*query.prepare(
        "SELECT id_usuario, nombre, email "
        "FROM usuarios"
        );*/

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

    return 0;
}

