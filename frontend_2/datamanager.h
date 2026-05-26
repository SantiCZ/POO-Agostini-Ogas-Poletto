#pragma once
#include <QObject>
#include <QVector>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonObject>
#include "models.h"
#include "admindb.h"

class DataManager : public QObject {
    Q_OBJECT
public:
    static DataManager& instance();

    // nuevo: id del usuario activo en sqlite
    int getUsuarioActivoId();

    // nuevo: login real contra el vps (asincrono)
    // responde con loginExitoso() o loginFallido()
    void loginRed(const QString& email, const QString& password);

    // sincronizacion completa desde el vps para un usuario
    void sincronizarDesdeServidor(int id_usuario);

    // expone la conexion sqlite activa a las vistas
    QSqlDatabase getDB();

    // tickets
    QVector<Ticket> getTickets(const QString& categoriaFiltro = "", const QString& busqueda = "");
    bool addTicket(const Ticket& t);
    bool removeTicket(int id);

    // suscripciones
    QVector<Suscripcion> getSuscripciones();
    bool addSuscripcion(const Suscripcion& s);
    bool updateSuscripcionEstado(int id, bool activa);
    bool removeSuscripcion(int id);

    // usuarios (compatibilidad con logindialog)
    bool addUser(const QString& username, const QString& password);
    bool login(const QString& username, const QString& password);
    bool userExists(const QString& username);
    void migrateUsers() {}

    // estadisticas
    double getGastoMes(int year, int month);
    int getTicketCountMes(int year, int month);
    int getSuscripcionesActivas();
    QVector<QPair<QString, double>> getGastosPorCategoria(int year, int month);
    QVector<QPair<QString, double>> getGastosPorSemana(int year, int month);

    // red
    enum EstadoRed { ESPERANDO, ENVIANDO_FOTO, SINCRONIZANDO, EXITO, ERROR_CONEXION };

    void analizarTicketRed(const QString& rutaImagen);
    void guardarTicketCompletoServidor(const QJsonObject& jsonCompleto);
    void registrarUsuarioRed(const QString& username, const QString& email, const QString& password);
    void guardarSuscripcionRed(const Suscripcion& s);
    void cambiarEstadoRed(EstadoRed nuevoEstado);

signals:
    // nuevo: resultado del login contra el vps
    void loginExitoso(int idUsuario, const QString& nombre);
    void loginFallido(const QString& mensaje);

    void estadoRedCambiado(DataManager::EstadoRed estado);
    void errorDeRed(const QString& mensaje);
    void ticketProcesadoRed(const QString& comercio, double monto, const QString& fecha, const QString& categoria, const QJsonObject& jsonCompleto);
    void ticketGuardadoServidor(bool exito, const QString& mensaje);
    void usuarioRegistradoServidor(bool exito, const QString& mensaje);
    void suscripcionGuardadaServidor(bool exito, const QString& mensaje);
    void sincronizacionCompletada();
    void ticketsChanged();
    void suscripcionesChanged();

private slots:
    void onRespuestaRecibida(QNetworkReply* reply);

private:
    DataManager();

    QString hashPassword(const QString& pwd) const;

    QNetworkAccessManager* networkManager;
    EstadoRed estadoActual = ESPERANDO;

    adminDB  m_db;
    QString  m_rutaDB;

    // almacena temporalmente los datos del usuario mientras llega el sync
    int     m_pendingUserId   = -1;
    QString m_pendingUsername;
};