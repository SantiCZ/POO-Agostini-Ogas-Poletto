#ifndef ADMINDB_H
#define ADMINDB_H

#include <QSqlDatabase>// Permite trabajar con bases de datos en Qt
#include <QString>// Permite usar QString
#include <QObject>// Clase base de Qt para objetos

class adminDB : public QObject
{
    Q_OBJECT

public:

    // Constructor de la clase
    explicit adminDB(QObject *parent = nullptr);

    // Función para conectar la base SQLite
    // Recibe la ruta del archivo .db
    bool conectar(QString archivoSqlite);

    // Devuelve el objeto de base de datos para poder usarlo en consultas
    QSqlDatabase getDB();

    // Función para validar usuario y contraseña
    bool validarUsuario(QString email, QString clave);

private:

    // Variable que representa la conexión con la base de datos SQLite
    QSqlDatabase db;
};

#endif // ADMINDB_H
