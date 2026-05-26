#include "admindb.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// nuevo: nombre fijo de conexion compartida por toda la app
// antes se usaba addDatabase("QSQLITE") sin nombre, lo que creaba
// qt_sql_default_connection y la destruia cada vez que se instanciaba adminDB
const QString adminDB::CONNECTION_NAME = "alcancia_connection";

// CONSTRUCTOR
adminDB::adminDB(QObject *parent) : QObject{parent}
{
    // nuevo: solo creamos la conexion si todavia no existe con ese nombre
    // antes: db = QSqlDatabase::addDatabase("QSQLITE");
    // eso reemplazaba la conexion anterior cada vez que se creaba un adminDB
    if (!QSqlDatabase::contains(CONNECTION_NAME))
    {
        db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    }
    else
    {
        // reutilizamos la conexion ya existente
        db = QSqlDatabase::database(CONNECTION_NAME);
    }
}

// CONECTAR BASE DE DATOS
bool adminDB::conectar(QString archivoSqlite)
{
    // si ya esta abierta y apunta al mismo archivo, no hacemos nada
    if (db.isOpen() && db.databaseName() == archivoSqlite)
    {
        return true;
    }

    db.setDatabaseName(archivoSqlite);

    if (db.open())
    {
        QSqlQuery query(db);
        query.exec("PRAGMA foreign_keys = OFF;");

        // nuevo: creamos las tablas locales que necesita la app si no existen.
        // estas replican el schema del servidor mysql adaptado a sqlite.
        // la tabla usuario_sesion es propia del cliente (no existe en mysql)
        // y guarda la sesion activa mas el hash de password para login offline.

        query.exec(R"(
            CREATE TABLE IF NOT EXISTS usuario_sesion (
                id_local          INTEGER PRIMARY KEY AUTOINCREMENT,
                id_usuario_remoto INTEGER,
                nombre            TEXT,
                email             TEXT,
                password_hash     TEXT,
                sesion_activa     INTEGER DEFAULT 1
            )
        )");

        query.exec(R"(
            CREATE TABLE IF NOT EXISTS categorias (
                id_categoria INTEGER PRIMARY KEY,
                id_usuario   INTEGER,
                nombre       TEXT NOT NULL,
                icono        TEXT,
                color        TEXT
            )
        )");

        query.exec(R"(
            CREATE TABLE IF NOT EXISTS gastos (
                id_gasto      INTEGER PRIMARY KEY,
                id_usuario    INTEGER,
                id_comprobante INTEGER,
                id_categoria  INTEGER,
                comercio      TEXT,
                monto         REAL NOT NULL,
                fecha_gasto   TEXT NOT NULL,
                notas         TEXT,
                fecha_registro TEXT
            )
        )");

        query.exec(R"(
            CREATE TABLE IF NOT EXISTS suscripciones (
                id_suscripcion INTEGER PRIMARY KEY,
                id_usuario     INTEGER,
                id_categoria   INTEGER,
                nombre         TEXT NOT NULL,
                monto          REAL NOT NULL,
                moneda         TEXT DEFAULT 'ARS',
                frecuencia     TEXT DEFAULT 'mensual',
                vencimiento    TEXT NOT NULL,
                alerta         INTEGER DEFAULT 3,
                actividad      INTEGER DEFAULT 1,
                notas          TEXT
            )
        )");

        query.exec(R"(
            CREATE TABLE IF NOT EXISTS notificaciones (
                id_notificacion INTEGER PRIMARY KEY,
                id_usuario      INTEGER,
                tipo            TEXT,
                mensaje         TEXT,
                leida           INTEGER DEFAULT 0,
                fecha_creacion  TEXT
            )
        )");

        // agregamos password_hash si la tabla ya existia sin esa columna
        // sqlite ignora el error si la columna ya existe
        query.exec("ALTER TABLE usuario_sesion ADD COLUMN password_hash TEXT;");

        return true;
    }

    qDebug() << "Error al abrir la base:" << db.lastError().text();
    return false;
}

QSqlDatabase adminDB::getDB()
{
    // nuevo: siempre devolvemos la conexion por nombre, no la copia local
    // esto garantiza que si alguien llama getDB() desde cualquier instancia
    // de adminDB, siempre obtiene la misma conexion activa
    return QSqlDatabase::database(CONNECTION_NAME);
}

// limpia todas las tablas antes de sincronizar datos nuevos (evita mezclar usuarios)
// ============================
// adminDB::limpiarBaseDeDatos()
// CORREGIDO
// ============================

bool adminDB::limpiarBaseDeDatos()
{
    QSqlDatabase conn =
        QSqlDatabase::database(CONNECTION_NAME);

    QSqlQuery query(conn);

    // IMPORTANTE:
    // NO borrar usuario_sesion
    // porque se pierde el usuario activo
    // y el password_hash offline

    int usuarioId = -1;

    QSqlQuery qUser(conn);

    qUser.prepare(R"(
        SELECT id_usuario_remoto
        FROM usuario_sesion
        WHERE sesion_activa = 1
        LIMIT 1
    )");

    if (qUser.exec() && qUser.next())
    {
        usuarioId = qUser.value(0).toInt();
    }

    if (usuarioId <= 0)
    {
        qDebug()
        << "limpiarBaseDeDatos:"
        << "no hay usuario activo";

        return false;
    }

    query.prepare(R"(
        DELETE FROM gastos
        WHERE id_usuario = :uid
    )");
    query.bindValue(":uid", usuarioId);
    query.exec();

    query.prepare(R"(
        DELETE FROM suscripciones
        WHERE id_usuario = :uid
    )");
    query.bindValue(":uid", usuarioId);
    query.exec();

    query.prepare(R"(
        DELETE FROM categorias
        WHERE id_usuario = :uid
    )");
    query.bindValue(":uid", usuarioId);
    query.exec();

    query.prepare(R"(
        DELETE FROM notificaciones
        WHERE id_usuario = :uid
    )");
    query.bindValue(":uid", usuarioId);
    query.exec();

    return true;
}

bool adminDB::sincronizarDesdeJson(QByteArray datosJson)
{
    QJsonParseError errorJson;
    QJsonDocument doc = QJsonDocument::fromJson(datosJson, &errorJson);

    if (errorJson.error != QJsonParseError::NoError || !doc.isObject())
    {
        qDebug() << "Error en formato JSON";
        return false;
    }

    QJsonObject root = doc.object();
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    conn.transaction();

    if (!limpiarBaseDeDatos())
    {
        conn.rollback();
        return false;
    }

    // nuevo: log detallado por paso para identificar exactamente que tabla falla
    bool ok = true;

    qDebug() << "sync: guardando usuario...";
    ok = guardarUsuarioSesion(root["usuario"].toObject());
    if (!ok) { qDebug() << "sync FALLO: guardarUsuarioSesion"; conn.rollback(); return false; }

    qDebug() << "sync: guardando categorias...";
    ok = guardarCategorias(root["categorias"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarCategorias"; conn.rollback(); return false; }

    qDebug() << "sync: guardando gastos...";
    ok = guardarGastos(root["gastos"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarGastos"; conn.rollback(); return false; }

    qDebug() << "sync: guardando suscripciones...";
    ok = guardarSuscripciones(root["suscripciones"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarSuscripciones"; conn.rollback(); return false; }

    qDebug() << "sync: guardando notificaciones...";
    ok = guardarNotificaciones(root["notificaciones"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarNotificaciones"; conn.rollback(); return false; }

    conn.commit();
    qDebug() << "Sincronizacion local exitosa (base limpia).";
    return true;
}

bool adminDB::guardarUsuarioSesion(QJsonObject usuario)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(conn);

    // nuevo: INSERT OR REPLACE para no fallar si ya habia una fila previa.
    // conservamos password_hash existente para no perder el login offline.
    // codigo anterior (conservado como referencia):
    // query.prepare("INSERT INTO usuario_sesion (id_usuario_remoto, nombre, email, sesion_activa) VALUES (:id, :nombre, :email, 1)");

    // primero obtenemos el hash guardado para no pisarlo
    QSqlQuery hashQuery(conn);
    hashQuery.exec("SELECT password_hash FROM usuario_sesion WHERE sesion_activa = 1");
    QString hashExistente = hashQuery.next() ? hashQuery.value(0).toString() : "";

    query.prepare(
        "INSERT OR REPLACE INTO usuario_sesion "
        "(id_usuario_remoto, nombre, email, password_hash, sesion_activa) "
        "VALUES (:id, :nombre, :email, :hash, 1)"
        );
    query.bindValue(":id",     usuario["id_usuario"].toInt());
    query.bindValue(":nombre", usuario["nombre"].toString());
    query.bindValue(":email",  usuario["email"].toString());
    query.bindValue(":hash",   hashExistente);

    if (!query.exec())
    {
        qDebug() << "guardarUsuarioSesion error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool adminDB::guardarCategorias(QJsonArray categorias)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (QJsonValue valor : categorias)
    {
        QJsonObject cat = valor.toObject();
        QSqlQuery query(conn);

        query.prepare(
            "INSERT OR REPLACE INTO categorias "
            "(id_categoria, id_usuario, nombre, icono, color) "
            "VALUES (:id, :uid, :nom, :ico, :col)"
            );

        query.bindValue(":id",  cat["id_categoria"].toInt());
        query.bindValue(":uid", cat["id_usuario"].toInt());
        query.bindValue(":nom", cat["nombre"].toString());
        query.bindValue(":ico", cat["icono"].toString());
        query.bindValue(":col", cat["color"].toString());

        if (!query.exec())
        {
            qDebug()
            << "guardarCategorias error:"
            << query.lastError().text()
            << "| datos:" << cat;

            return false;
        }
    }

    return true;
}
bool adminDB::guardarGastos(QJsonArray gastos)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (QJsonValue valor : gastos)
    {
        QJsonObject gasto = valor.toObject();
        QSqlQuery query(conn);

        // nuevo: columnas reales del schema (id_gasto, id_categoria, sin sufijo _remota)
        // codigo anterior incorrecto (conservado como referencia):
        // INSERT INTO gastos (id_gasto_remoto, id_categoria_remota, ..., sincronizado)
        query.prepare(
            "INSERT OR REPLACE INTO gastos "
            "(id_gasto, id_usuario, id_comprobante, id_categoria, "
            " comercio, monto, fecha_gasto, notas) "
            "VALUES (:id, :uid, :comp, :cat, :com, :mon, :fec, :not)"
            );
        query.bindValue(":id",   gasto["id_gasto"].toInt());
        query.bindValue(":uid",  gasto["id_usuario"].toInt());
        query.bindValue(":comp", gasto["id_comprobante"].toInt());
        query.bindValue(":cat",  gasto["id_categoria"].toInt());
        query.bindValue(":com",  gasto["comercio"].toString());
        query.bindValue(":mon",  gasto["monto"].toDouble());
        query.bindValue(":fec",  gasto["fecha_gasto"].toString());
        query.bindValue(":not",  gasto["notas"].toString());

        if (!query.exec())
        {
            qDebug() << "guardarGastos error:" << query.lastError().text()
            << "| datos:" << gasto;
            return false;
        }
    }

    return true;
}

bool adminDB::guardarSuscripciones(QJsonArray suscripciones)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (QJsonValue valor : suscripciones)
    {
        QJsonObject sub = valor.toObject();
        QSqlQuery query(conn);

        // nuevo: usamos las columnas reales del schema (nombre_servicio, fecha_vencimiento)
        // codigo anterior incorrecto (conservado como referencia):
        // query.prepare("INSERT INTO suscripciones (id_suscripcion_remota, nombre, monto, vencimiento, sincronizado) ...");
        // nuevo: columnas reales del schema mysql (nombre, vencimiento, actividad, alerta)
        // codigo anterior incorrecto (conservado como referencia):
        // INSERT INTO suscripciones (id_suscripcion_remota, nombre_servicio, fecha_vencimiento...)
        query.prepare(
            "INSERT OR REPLACE INTO suscripciones "
            "(id_suscripcion, id_usuario, id_categoria, nombre, monto, "
            " moneda, frecuencia, vencimiento, alerta, actividad, notas) "
            "VALUES (:id, :uid, :cat, :nom, :mon, :mon2, :frec, :venc, :alerta, :activ, :notas)"
            );
        query.bindValue(":id",     sub["id_suscripcion"].toInt());
        query.bindValue(":uid",    sub["id_usuario"].toInt());
        query.bindValue(":cat",    sub["id_categoria"].toInt());
        query.bindValue(":nom",    sub["nombre"].toString());
        query.bindValue(":mon",    sub["monto"].toDouble());
        query.bindValue(":mon2",   sub["moneda"].toString());
        query.bindValue(":frec",   sub["frecuencia"].toString());
        query.bindValue(":venc",   sub["vencimiento"].toString());
        query.bindValue(":alerta", sub["alerta"].toInt());
        query.bindValue(":activ",  sub["actividad"].toInt());
        query.bindValue(":notas",  sub["notas"].toString());

        if (!query.exec())
        {
            qDebug() << "guardarSuscripciones error:" << query.lastError().text()
            << "| datos:" << sub;
            return false;
        }
    }

    return true;
}

bool adminDB::guardarNotificaciones(QJsonArray notificaciones)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (QJsonValue valor : notificaciones)
    {
        QJsonObject notif = valor.toObject();
        QSqlQuery query(conn);

        query.prepare(
            "INSERT OR REPLACE INTO notificaciones "
            "(id_notificacion, id_usuario, tipo, mensaje, leida, fecha_creacion) "
            "VALUES (:id, :uid, :tip, :men, :lei, :fec)"
            );

        query.bindValue(":id",  notif["id_notificacion"].toInt());
        query.bindValue(":uid", notif["id_usuario"].toInt());
        query.bindValue(":tip", notif["tipo"].toString());
        query.bindValue(":men", notif["mensaje"].toString());
        query.bindValue(":lei", notif["leida"].toInt());
        query.bindValue(":fec", notif["fecha_creacion"].toString());

        if (!query.exec())
        {
            qDebug()
            << "guardarNotificaciones error:"
            << query.lastError().text()
            << "| datos:" << notif;

            return false;
        }
    }

    return true;
}