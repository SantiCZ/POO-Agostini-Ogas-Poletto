#include "admindb.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

/*
 * admindb.cpp
 * Implementa la persistencia local SQLite.
 * Esta clase contiene el SQL y las reglas de sincronizacion local para que
 * DataManager y las pantallas no dependan del detalle de tablas.
 */

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
    // SQLite:
    // Aqui se abre/crea la base local. Todas las tablas se preparan desde esta
    // clase para que el resto del proyecto no dependa del SQL de bajo nivel.
    // Si la conexion ya apunta al archivo correcto, se reutiliza para evitar
    // cerrar la base mientras otras clases la estan usando.
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

        // usuario_sesion: tabla propia del cliente. Guarda quien esta activo y
        // permite login offline con password_hash si ya hubo una sesion previa.
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

        // categorias: catalogo local que permite clasificar gastos aunque la
        // UI este leyendo desde SQLite.
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS categorias (
                id_categoria INTEGER PRIMARY KEY,
                id_usuario   INTEGER,
                nombre       TEXT NOT NULL,
                icono        TEXT,
                color        TEXT
            )
        )");

        // gastos: copia local de tickets/gastos. Se consulta para dashboard,
        // reportes y listado de tickets.
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

        // suscripciones: mantiene estado local, actividad y marcas pendientes
        // para sincronizar creaciones/ediciones/eliminaciones.
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS suscripciones (
                id_suscripcion_local INTEGER PRIMARY KEY AUTOINCREMENT,
                id_suscripcion_remota INTEGER,
                id_usuario_remoto INTEGER NOT NULL,
                nombre TEXT NOT NULL,
                monto REAL NOT NULL,
                moneda TEXT DEFAULT 'ARS',
                frecuencia TEXT DEFAULT 'mensual',
                vencimiento DATE NOT NULL,
                alerta INTEGER DEFAULT 3,
                actividad INTEGER DEFAULT 1,
                notas TEXT,
                sincronizado INTEGER NOT NULL DEFAULT 0,
                accion_pendiente TEXT DEFAULT 'crear'
                    CHECK (accion_pendiente IN ('crear','editar','eliminar','ninguna')),
                fecha_sync DATETIME
            )
        )");

        // notificaciones: avisos generados desde vencimientos de suscripciones.
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
    // Solo se limpian datos del usuario activo para no mezclar informacion
    // de distintas sesiones despues de una sincronizacion completa.
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
        WHERE id_usuario_remoto = :uid
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
    // SQLite:
    // La sincronizacion usa una transaccion. Si falla guardar alguna tabla,
    // rollback evita dejar la base en un estado parcial.
    // El servidor entrega todo el estado del usuario en JSON. La transaccion
    // asegura que no quede una sincronizacion parcial si alguna tabla falla.
    QJsonParseError errorJson;
    QJsonDocument doc = QJsonDocument::fromJson(datosJson, &errorJson);

    if (errorJson.error != QJsonParseError::NoError || !doc.isObject())
    {
        qDebug() << "Error en formato JSON";
        return false;
    }

    QJsonObject root = doc.object();
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    // La transaccion agrupa limpieza + guardado. Si una parte falla, rollback
    // deja SQLite como estaba antes de empezar la sincronizacion.
    conn.transaction();

    if (!limpiarBaseDeDatos())
    {
        conn.rollback();
        return false;
    }

    bool ok = true;

    ok = guardarUsuarioSesion(root["usuario"].toObject());
    if (!ok) { qDebug() << "sync FALLO: guardarUsuarioSesion"; conn.rollback(); return false; }

    ok = guardarCategorias(root["categorias"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarCategorias"; conn.rollback(); return false; }

    ok = guardarGastos(root["gastos"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarGastos"; conn.rollback(); return false; }

    ok = guardarSuscripciones(root["suscripciones"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarSuscripciones"; conn.rollback(); return false; }

    ok = guardarNotificaciones(root["notificaciones"].toArray());
    if (!ok) { qDebug() << "sync FALLO: guardarNotificaciones"; conn.rollback(); return false; }

    conn.commit();
    return true;
}

bool adminDB::guardarUsuarioSesion(QJsonObject usuario)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    QSqlQuery limpiarSesion(conn);
    limpiarSesion.exec(R"(
        DELETE FROM usuario_sesion
    )");

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



bool adminDB::guardarCategorias(const QJsonArray &categorias)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (const QJsonValue &valor : categorias)
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

bool adminDB::guardarGastos(const QJsonArray &gastos)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (const QJsonValue &valor : gastos)
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

bool adminDB::guardarSuscripciones(const QJsonArray &suscripciones)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (const QJsonValue &valor : suscripciones)
    {
        QJsonObject sub = valor.toObject();
        QSqlQuery query(conn);

        query.prepare(R"(
            INSERT OR REPLACE INTO suscripciones
            (
                id_suscripcion_remota,
                id_usuario_remoto,
                nombre,
                monto,
                moneda,
                frecuencia,
                vencimiento,
                alerta,
                actividad,
                notas,
                sincronizado,
                accion_pendiente,
                fecha_sync
            )
            VALUES
            (
                :id_suscripcion,
                :id_usuario,
                :nombre,
                :monto,
                :moneda,
                :frecuencia,
                :vencimiento,
                :alerta,
                :actividad,
                :notas,
                1,
                'ninguna',
                datetime('now')
            )
        )");

        query.bindValue(
            ":id_suscripcion",
            sub["id_suscripcion"].toInt()
            );

        query.bindValue(
            ":id_usuario",
            sub["id_usuario"].toInt()
            );

        query.bindValue(
            ":nombre",
            sub["nombre"].toString()
            );

        query.bindValue(
            ":monto",
            sub["monto"].toDouble()
            );

        query.bindValue(
            ":moneda",
            sub["moneda"].toString()
            );

        query.bindValue(
            ":frecuencia",
            sub["frecuencia"].toString()
            );

        query.bindValue(
            ":vencimiento",
            sub["vencimiento"].toString()
            );

        query.bindValue(
            ":alerta",
            sub["alerta"].toInt()
            );

        query.bindValue(
            ":actividad",
            sub["actividad"].toInt()
            );

        query.bindValue(
            ":notas",
            sub["notas"].toString()
            );

        if (!query.exec())
        {
            qDebug()
            << "guardarSuscripciones error:"
            << query.lastError().text();

            qDebug()
                << "Consulta:"
                << query.lastQuery();

            return false;
        }

    }

    return true;
}

bool adminDB::guardarNotificaciones(const QJsonArray &notificaciones)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    for (const QJsonValue &valor : notificaciones)
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

bool adminDB::obtenerUltimoUsuario(QString &nombre, QString &email)
{
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    QSqlQuery query(conn);

    query.prepare(R"(
        SELECT nombre, email
        FROM usuario_sesion
        WHERE sesion_activa = 1
        LIMIT 1
    )");

    if (!query.exec())
    {
        qDebug() << "Error obteniendo ultimo usuario:"
                 << query.lastError().text();
        return false;
    }

    if (query.next())
    {
        nombre = query.value("nombre").toString();
        email = query.value("email").toString();
        return true;
    }

    return false;
}

bool adminDB::generarNotificacionesVencimiento()
{
    // Busca suscripciones cercanas al vencimiento y genera avisos locales,
    // evitando duplicados sin leer para no saturar al usuario.
    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    QSqlQuery buscar(conn);

    buscar.prepare(R"(
        SELECT id_usuario_remoto, nombre, vencimiento, alerta
        FROM suscripciones
        WHERE actividad = 1
        AND date(vencimiento) <= date('now', '+' || alerta || ' days')
    )");

    if (!buscar.exec())
    {
        qDebug() << "Error buscando vencimientos:"
                 << buscar.lastError().text();
        return false;
    }

    while (buscar.next())
    {
        int idUsuario = buscar.value("id_usuario_remoto").toInt();
        QString nombre = buscar.value("nombre").toString();
        QString vencimiento = buscar.value("vencimiento").toString();

        QString mensaje =
            "Tu suscripcion " + nombre +
            " vence el " + vencimiento;


        QSqlQuery existe(conn);

        existe.prepare(R"(
            SELECT COUNT(*)
            FROM notificaciones
            WHERE id_usuario = :uid
            AND tipo = 'vencimiento'
            AND mensaje = :mensaje
            AND leida = 0
        )");

        existe.bindValue(":uid", idUsuario);
        existe.bindValue(":mensaje", mensaje);

        if (!existe.exec())
        {
            qDebug() << "Error verificando duplicado:"
                     << existe.lastError().text();
            return false;
        }

        if (existe.next() && existe.value(0).toInt() > 0)
        {
            // Si ya existe una notificacion igual sin leer, no se duplica.
            continue;
        }

        QSqlQuery insertar(conn);

        insertar.prepare(R"(
            INSERT INTO notificaciones
            (id_usuario, tipo, mensaje, leida, fecha_creacion)
            VALUES (:uid, 'vencimiento', :mensaje, 0, datetime('now'))
        )");

        insertar.bindValue(":uid", idUsuario);
        insertar.bindValue(":mensaje", mensaje);

        if (!insertar.exec())
        {
            qDebug() << "Error creando notificacion:"
                     << insertar.lastError().text();
            return false;
        }
    }

    return true;
}

bool adminDB::renovarSuscripcionesVencidas()
{
    // Si una suscripcion ya vencio, calcula el siguiente vencimiento segun
    // frecuencia y la marca como pendiente de sincronizar.

    QSqlDatabase conn = QSqlDatabase::database(CONNECTION_NAME);

    QSqlQuery buscar(conn);

    buscar.prepare(R"(
        SELECT id_suscripcion_local,
               nombre,
               vencimiento,
               frecuencia
        FROM suscripciones
        WHERE actividad = 1
        AND date(vencimiento) < date('now')
    )");

    if (!buscar.exec())
    {
        return false;
    }

    while (buscar.next())
    {

        int idLocal = buscar.value("id_suscripcion_local").toInt();
        QString nombre = buscar.value("nombre").toString();
        QString vencimientoTexto = buscar.value("vencimiento").toString();
        QString frecuencia = buscar.value("frecuencia").toString().toLower();

        QDate vencimiento = QDate::fromString(vencimientoTexto, Qt::ISODate);
        QDate nuevoVencimiento = vencimiento;

        QDate hoy = QDate::currentDate();

        // Se avanza hasta una fecha futura. Esto cubre el caso de una
        // suscripcion vencida hace varios meses o semanas.
        if (frecuencia == "mensual")
        {
            while (nuevoVencimiento < hoy)
                nuevoVencimiento = nuevoVencimiento.addMonths(1);
        }
        else if (frecuencia == "semanal")
        {
            while (nuevoVencimiento < hoy)
                nuevoVencimiento = nuevoVencimiento.addDays(7);
        }
        else if (frecuencia == "anual")
        {
            while (nuevoVencimiento < hoy)
                nuevoVencimiento = nuevoVencimiento.addYears(1);
        }

        QSqlQuery actualizar(conn);

        actualizar.prepare(R"(
            UPDATE suscripciones
            SET vencimiento = :nuevoVencimiento,
                sincronizado = 0,
                accion_pendiente = 'editar'
            WHERE id_suscripcion_local = :id
        )");

        actualizar.bindValue(":nuevoVencimiento", nuevoVencimiento.toString(Qt::ISODate));
        actualizar.bindValue(":id", idLocal);

        if (!actualizar.exec())
        {
            return false;
        }

    }

    return true;
}
