#include "admindbproyecto.h"
#include <QSqlError> // Permite mostrar errores SQL
#include <QSqlQuery> // Permite ejecutar consultas SQL
#include <QDebug> // Permite imprimir mensajes en consola
#include <QFileInfo>

// CONSTRUCTOR
// ------------------------------------------------------
adminDBproyecto::adminDBproyecto(QObject *parent)
    : QObject{parent}
{
    // Creamos una conexión usando SQLite "QSQLITE" indica el motor de base de datos que vamos a usar
    db = QSqlDatabase::addDatabase("QSQLITE");
}
// CONECTAR BASE DE DATOS
//--------------------------------------------------------
bool adminDBproyecto::conectar(QString archivoSqlite)
{
    QFileInfo archivo(archivoSqlite);

    qDebug() << "Ruta recibida:" << archivoSqlite;
    qDebug() << "Ruta absoluta:" << archivo.absoluteFilePath();
    qDebug() << "Existe archivo:" << archivo.exists();

    db.setDatabaseName(archivoSqlite);

    if (db.open())
    {
        qDebug() << "Base abierta:" << db.databaseName();

        QSqlQuery query(db);
        query.exec("PRAGMA foreign_keys = ON;");

        return true;
    }

    qDebug() << "Error al abrir la base:" << db.lastError().text();
    return false;
}
// DEVOLVER LA CONEXIÓN
//-----------------------------------------------------------
QSqlDatabase adminDBproyecto::getDB()
{
    // Retorna la conexión actual para usarla desde otras partes
    return db;
}