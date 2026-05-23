#include "admindb.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// CONSTRUCTOR
adminDB::adminDB(QObject *parent) : QObject{parent} {
    db = QSqlDatabase::addDatabase("QSQLITE");
}

// CONECTAR BASE DE DATOS
bool adminDB::conectar(QString archivoSqlite) {
    db.setDatabaseName(archivoSqlite);
    if (db.open()) {
        QSqlQuery query(db);
        query.exec("PRAGMA foreign_keys = ON;");
        return true;
    }
    qDebug() << "Error al abrir la base:" << db.lastError().text();
    return false;
}

QSqlDatabase adminDB::getDB() { return db; }

// NUEVO: Limpia todas las tablas antes de sincronizar datos nuevos (evita mezclar usuarios)
bool adminDB::limpiarBaseDeDatos() {
    QSqlQuery query(db);
    // Borramos datos de todas las tablas relevantes
    query.exec("DELETE FROM gastos;");
    query.exec("DELETE FROM suscripciones;");
    query.exec("DELETE FROM categorias;");
    query.exec("DELETE FROM notificaciones;");
    query.exec("DELETE FROM usuario_sesion;");
    return true;
}

bool adminDB::sincronizarDesdeJson(QByteArray datosJson) {
    QJsonParseError errorJson;
    QJsonDocument doc = QJsonDocument::fromJson(datosJson, &errorJson);

    if (errorJson.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "Error en formato JSON";
        return false;
    }

    QJsonObject root = doc.object();
    db.transaction();

    // CORRECCIÓN: Antes de insertar, limpiamos la base para el usuario nuevo
    if (!limpiarBaseDeDatos()) {
        db.rollback();
        return false;
    }

    bool ok = true;
    ok = ok && guardarUsuarioSesion(root["usuario"].toObject());
    ok = ok && guardarCategorias(root["categorias"].toArray());
    ok = ok && guardarGastos(root["gastos"].toArray());
    ok = ok && guardarSuscripciones(root["suscripciones"].toArray());
    ok = ok && guardarNotificaciones(root["notificaciones"].toArray());

    if (ok) {
        db.commit();
        qDebug() << "Sincronización local exitosa (base limpia).";
        return true;
    } else {
        db.rollback();
        qDebug() << "Error en la sincronización, cambios revertidos.";
        return false;
    }
}

// MÉTODOS DE GUARDADO (Se mantienen igual, pero aseguran integridad)
bool adminDB::guardarUsuarioSesion(QJsonObject usuario) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO usuario_sesion (id_usuario_remoto, nombre, email, sesion_activa) VALUES (:id, :nombre, :email, 1)");
    query.bindValue(":id", usuario["id_usuario"].toInt());
    query.bindValue(":nombre", usuario["nombre"].toString());
    query.bindValue(":email", usuario["email"].toString());
    return query.exec();
}

bool adminDB::guardarCategorias(QJsonArray categorias) {
    for (QJsonValue valor : categorias) {
        QJsonObject cat = valor.toObject();
        QSqlQuery query(db);
        query.prepare("INSERT INTO categorias (id_categoria_remota, nombre, icono, color, sincronizado) VALUES (:id, :nom, :ico, :col, 1)");
        query.bindValue(":id", cat["id_categoria"].toInt());
        query.bindValue(":nom", cat["nombre"].toString());
        query.bindValue(":ico", cat["icono"].toString());
        query.bindValue(":col", cat["color"].toString());
        if (!query.exec()) return false;
    }
    return true;
}

bool adminDB::guardarGastos(QJsonArray gastos) {
    for (QJsonValue valor : gastos) {
        QJsonObject gasto = valor.toObject();
        QSqlQuery query(db);
        query.prepare("INSERT INTO gastos (id_gasto_remoto, id_categoria_remota, comercio, monto, fecha_gasto, notas, sincronizado) VALUES (:id, :cat, :com, :mon, :fec, :not, 1)");
        query.bindValue(":id", gasto["id_gasto"].toInt());
        query.bindValue(":cat", gasto["id_categoria"].toInt());
        query.bindValue(":com", gasto["comercio"].toString());
        query.bindValue(":mon", gasto["monto"].toDouble());
        query.bindValue(":fec", gasto["fecha_gasto"].toString());
        query.bindValue(":not", gasto["notas"].toString());
        if (!query.exec()) return false;
    }
    return true;
}

bool adminDB::guardarSuscripciones(QJsonArray suscripciones) {
    for (QJsonValue valor : suscripciones) {
        QJsonObject sub = valor.toObject();
        QSqlQuery query(db);
        query.prepare("INSERT INTO suscripciones (id_suscripcion_remota, nombre, monto, vencimiento, sincronizado) VALUES (:id, :nom, :mon, :venc, 1)");
        query.bindValue(":id", sub["id_suscripcion"].toInt());
        query.bindValue(":nom", sub["nombre"].toString());
        query.bindValue(":mon", sub["monto"].toDouble());
        query.bindValue(":venc", sub["vencimiento"].toString());
        if (!query.exec()) return false;
    }
    return true;
}

bool adminDB::guardarNotificaciones(QJsonArray notificaciones) {
    for (QJsonValue valor : notificaciones) {
        QJsonObject notif = valor.toObject();
        QSqlQuery query(db);
        query.prepare("INSERT INTO notificaciones (id_notificacion_remota, tipo, mensaje, leida, sincronizado) VALUES (:id, :tip, :men, :lei, 1)");
        query.bindValue(":id", notif["id_notificacion"].toInt());
        query.bindValue(":tip", notif["tipo"].toString());
        query.bindValue(":men", notif["mensaje"].toString());
        query.bindValue(":lei", notif["leida"].toInt());
        if (!query.exec()) return false;
    }
    return true;
}