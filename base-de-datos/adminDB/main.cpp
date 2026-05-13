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
    QSqlQuery query(dbProyecto.getDB());


    // Preparamos la consulta SQL
    query.prepare(
        "SELECT id_usuario, nombre, email "
        "FROM usuarios"
        );


    // Ejecutamos la consulta
    if (query.exec())
    {
        // Mientras existan filas
        while (query.next())
        {
            // Leemos los datos de la fila actual

            qDebug() << "ID:"
                     << query.value("id_usuario").toInt();

            qDebug() << "Nombre:"
                     << query.value("nombre").toString();

            qDebug() << "Email:"
                     << query.value("email").toString();
        }
    }
    else
    {
        // Si falla la consulta
        qDebug() << "Error en SELECT:"
                 << query.lastError().text();
    }


    return 0;
}