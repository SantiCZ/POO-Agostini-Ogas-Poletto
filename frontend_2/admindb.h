#ifndef ADMINDB_H
#define ADMINDB_H

#include <QObject>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>

class adminDB : public QObject
{
    Q_OBJECT

public:
    explicit adminDB(QObject *parent = nullptr);

    bool conectar(QString archivoSqlite);
    QSqlDatabase getDB();

    bool limpiarBaseDeDatos();
    bool sincronizarDesdeJson(QByteArray datosJson);

    bool guardarUsuarioSesion(QJsonObject usuario);
    bool guardarCategorias(QJsonArray categorias);
    bool guardarGastos(QJsonArray gastos);
    bool guardarSuscripciones(QJsonArray suscripciones);
    bool obtenerUltimoUsuario(QString &nombre, QString &email);
    bool guardarNotificaciones(QJsonArray notificaciones);
    bool generarNotificacionesVencimiento();
    bool renovarSuscripcionesVencidas();

private:
    // nuevo: nombre fijo de conexion para que todos reutilicen la misma
    // antes se usaba el nombre por defecto (qt_sql_default_connection) que
    // se destruia cada vez que se creaba un nuevo adminDB en cualquier lugar
    static const QString CONNECTION_NAME;

    QSqlDatabase db;
};

#endif // ADMINDB_H