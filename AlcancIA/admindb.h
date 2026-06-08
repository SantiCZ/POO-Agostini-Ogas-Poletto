#ifndef ADMINDB_H
#define ADMINDB_H

#include <QObject>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>

/*
 * adminDB
 * Responsabilidad de clase:
 * Encapsula la conexion y las operaciones directas contra SQLite.
 * La interfaz y el resto de la app no ejecutan SQL directamente; delegan en
 * esta clase para mantener separada la persistencia local.
 *
 * Herencia:
 * Hereda de QObject para integrarse con Qt y permitir propiedad de objetos.
 *
 * Clases abstractas:
 * No es abstracta: contiene implementaciones concretas de conexion, creacion
 * de tablas y consultas SQL.
 */
class adminDB : public QObject
{
    Q_OBJECT

public:
    explicit adminDB(QObject *parent = nullptr);

    // Abre o crea la base local y prepara las tablas necesarias.
    bool conectar(QString archivoSqlite);

    // Devuelve la conexion compartida para consultas puntuales.
    QSqlDatabase getDB();

    // Limpia datos del usuario activo antes de cargar una nueva sincronizacion.
    bool limpiarBaseDeDatos();

    // Convierte el JSON recibido del VPS en registros SQLite locales.
    bool sincronizarDesdeJson(QByteArray datosJson);

    // Guardado por tipo de entidad. Cada metodo conoce el esquema de su tabla.
    bool guardarUsuarioSesion(QJsonObject usuario);
    bool guardarCategorias(const QJsonArray &categorias);
    bool guardarGastos(const QJsonArray &gastos);
    bool guardarSuscripciones(const QJsonArray &suscripciones);
    bool guardarNotificaciones(const QJsonArray &notificaciones);

    // Recupera datos de sesion y mantiene alertas de vencimientos.
    bool obtenerUltimoUsuario(QString &nombre, QString &email);
    bool generarNotificacionesVencimiento();
    bool renovarSuscripcionesVencidas();

private:
    // Atributo importante: nombre fijo de conexion SQLite.
    // Evita que distintas instancias creen conexiones separadas e inconsistentes.
    static const QString CONNECTION_NAME;

    // Atributo importante: objeto de conexion local a SQLite.
    QSqlDatabase db;
};

#endif // ADMINDB_H
