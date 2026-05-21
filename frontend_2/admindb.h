#ifndef ADMINDB_H
#define ADMINDB_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>

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

    bool sincronizarDesdeJson(QByteArray datosJson);

    bool guardarUsuarioSesion(QJsonObject usuario);
    bool guardarCategorias(QJsonArray categorias);
    bool guardarGastos(QJsonArray gastos);
    bool guardarSuscripciones(QJsonArray suscripciones);
    bool guardarNotificaciones(QJsonArray notificaciones);

private:
    QSqlDatabase db;
};

#endif // ADMINDB_H