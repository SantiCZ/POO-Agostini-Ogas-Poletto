#include "admindb.h"
#include <QSqlError> // Permite mostrar errores SQL
#include <QSqlQuery> // Permite ejecutar consultas SQL
#include <QDebug> // Permite imprimir mensajes en consola
#include <QFileInfo>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// CONSTRUCTOR
// ------------------------------------------------------
adminDB::adminDB(QObject *parent)
    : QObject{parent}
{
    // Creamos una conexión usando SQLite "QSQLITE" indica el motor de base de datos que vamos a usar
    db = QSqlDatabase::addDatabase("QSQLITE");
}
// CONECTAR BASE DE DATOS
//--------------------------------------------------------
bool adminDB::conectar(QString archivoSqlite)
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
QSqlDatabase adminDB::getDB()
{
    // Retorna la conexión actual para usarla desde otras partes
    return db;
}

bool adminDB::sincronizarDesdeJson(QByteArray datosJson)
{
    QJsonParseError errorJson;

    QJsonDocument doc = QJsonDocument::fromJson(datosJson, &errorJson);

    if (errorJson.error != QJsonParseError::NoError)
    {
        qDebug() << "Error leyendo JSON:" << errorJson.errorString();
        return false;
    }

    if (!doc.isObject())
    {
        qDebug() << "El JSON no tiene formato de objeto principal";
        return false;
    }

    QJsonObject root = doc.object();

    qDebug() << "JSON leído correctamente";

    db.transaction();

    bool ok = true;

    ok = ok && guardarUsuarioSesion(root["usuario"].toObject());
    ok = ok && guardarCategorias(root["categorias"].toArray());
    ok = ok && guardarGastos(root["gastos"].toArray());
    ok = ok && guardarSuscripciones(root["suscripciones"].toArray());
    ok = ok && guardarNotificaciones(root["notificaciones"].toArray());

    if (ok)
    {
        db.commit();
        qDebug() << "Sincronización local finalizada correctamente";
        return true;
    }
    else
    {
        db.rollback();
        qDebug() << "Error en la sincronización. Se cancelaron los cambios.";
        return false;
    }
}

bool adminDB::guardarUsuarioSesion(QJsonObject usuario)
{
    QSqlQuery query(db);

    query.prepare(
        "INSERT INTO usuario_sesion "
        "(id_usuario_remoto, nombre, email, sesion_activa) "
        "VALUES (:id_usuario_remoto, :nombre, :email, 1)"
        );

    query.bindValue(":id_usuario_remoto", usuario["id_usuario"].toInt());
    query.bindValue(":nombre", usuario["nombre"].toString());
    query.bindValue(":email", usuario["email"].toString());

    if (!query.exec())
    {
        qDebug() << "Error guardando usuario_sesion:" << query.lastError().text();
        return false;
    }

    qDebug() << "Usuario de sesión guardado";
    return true;
}

bool adminDB::guardarCategorias(QJsonArray categorias)
{
    for (QJsonValue valor : categorias)
    {
        QJsonObject categoria = valor.toObject();

        QSqlQuery query(db);

        query.prepare(
            "INSERT INTO categorias "
            "(id_categoria_remota, nombre, icono, color, sincronizado) "
            "VALUES (:id_categoria_remota, :nombre, :icono, :color, 1)"
            );

        query.bindValue(":id_categoria_remota", categoria["id_categoria"].toInt());
        query.bindValue(":nombre", categoria["nombre"].toString());
        query.bindValue(":icono", categoria["icono"].toString());
        query.bindValue(":color", categoria["color"].toString());

        if (!query.exec())
        {
            qDebug() << "Error guardando categoria:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Categorías guardadas";
    return true;
}

bool adminDB::guardarGastos(QJsonArray gastos)
{
    for (QJsonValue valor : gastos)
    {
        QJsonObject gasto = valor.toObject();

        QSqlQuery query(db);

        query.prepare(
            "INSERT INTO gastos "
            "(id_gasto_remoto, id_usuario_remoto, id_categoria_remota, comercio, monto, fecha_gasto, notas, sincronizado, accion_pendiente) "
            "VALUES (:id_gasto_remoto, :id_usuario_remoto, :id_categoria_remota, :comercio, :monto, :fecha_gasto, :notas, 1, 'ninguna')"
            );

        query.bindValue(":id_gasto_remoto", gasto["id_gasto"].toInt());
        query.bindValue(":id_usuario_remoto", 1);
        query.bindValue(":id_categoria_remota", gasto["id_categoria"].toInt());
        query.bindValue(":comercio", gasto["comercio"].toString());
        query.bindValue(":monto", gasto["monto"].toDouble());
        query.bindValue(":fecha_gasto", gasto["fecha_gasto"].toString());
        query.bindValue(":notas", gasto["notas"].toString());

        if (!query.exec())
        {
            qDebug() << "Error guardando gasto:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Gastos guardados";
    return true;
}

bool adminDB::guardarSuscripciones(QJsonArray suscripciones)
{
    for (QJsonValue valor : suscripciones)
    {
        QJsonObject suscripcion = valor.toObject();

        QSqlQuery query(db);

        query.prepare(
            "INSERT INTO suscripciones "
            "(id_suscripcion_remota, id_usuario_remoto, id_categoria_remota, nombre, monto, moneda, frecuencia, vencimiento, alerta, actividad, notas, sincronizado, accion_pendiente) "
            "VALUES (:id_suscripcion_remota, :id_usuario_remoto, :id_categoria_remota, :nombre, :monto, :moneda, :frecuencia, :vencimiento, :alerta, :actividad, :notas, 1, 'ninguna')"
            );

        query.bindValue(":id_suscripcion_remota", suscripcion["id_suscripcion"].toInt());
        query.bindValue(":id_usuario_remoto", 1);
        query.bindValue(":id_categoria_remota", suscripcion["id_categoria"].toInt());
        query.bindValue(":nombre", suscripcion["nombre"].toString());
        query.bindValue(":monto", suscripcion["monto"].toDouble());
        query.bindValue(":moneda", suscripcion["moneda"].toString());
        query.bindValue(":frecuencia", suscripcion["frecuencia"].toString());
        query.bindValue(":vencimiento", suscripcion["vencimiento"].toString());
        query.bindValue(":alerta", suscripcion["alerta"].toInt());
        query.bindValue(":actividad", suscripcion["actividad"].toInt());
        query.bindValue(":notas", suscripcion["notas"].toString());

        if (!query.exec())
        {
            qDebug() << "Error guardando suscripcion:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Suscripciones guardadas";
    return true;
}

bool adminDB::guardarNotificaciones(QJsonArray notificaciones)
{
    for (QJsonValue valor : notificaciones)
    {
        QJsonObject notificacion = valor.toObject();

        QSqlQuery query(db);

        query.prepare(
            "INSERT INTO notificaciones "
            "(id_notificacion_remota, id_usuario_remoto, tipo, mensaje, leida, sincronizado) "
            "VALUES (:id_notificacion_remota, :id_usuario_remoto, :tipo, :mensaje, :leida, 1)"
            );

        query.bindValue(":id_notificacion_remota", notificacion["id_notificacion"].toInt());
        query.bindValue(":id_usuario_remoto", 1);
        query.bindValue(":tipo", notificacion["tipo"].toString());
        query.bindValue(":mensaje", notificacion["mensaje"].toString());
        query.bindValue(":leida", notificacion["leida"].toInt());

        if (!query.exec())
        {
            qDebug() << "Error guardando notificacion:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Notificaciones guardadas";
    return true;
}

