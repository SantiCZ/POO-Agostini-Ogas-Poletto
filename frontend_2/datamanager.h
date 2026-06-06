#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QSqlDatabase>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonObject>

#include "models.h"
#include "admindb.h"

class DataManager : public QObject {
    Q_OBJECT
public:
    static DataManager& instance();

    // Último email recordado
    QString getUltimoEmail() const;
    void setUltimoEmail(const QString &email);

    // Notificaciones
    QVector<Notificacion> getNotificaciones() const;
    void agregarNotificacion(const Notificacion &n);
    void cargarNotificacionesDesdeSQLite();

    // Suscripciones (solo una declaración)
    bool updateSuscripcion(const Suscripcion &s);

    // resto de métodos existentes...
    int getUsuarioActivoId();
    void loginRed(const QString& email, const QString& password);
    void sincronizarDesdeServidor(int id_usuario);
    void sincronizarSuscripcionesLocales(); // CAMBIO NUEVO - sincronización SQLite → VPS
    QSqlDatabase getDB();

    QVector<Ticket> getTickets(const QString& categoriaFiltro = "", const QString& busqueda = "");
    bool addTicket(const Ticket& t);
    bool removeTicket(int id);

    QVector<Suscripcion> getSuscripciones();
    bool addSuscripcion(const Suscripcion& s);
    bool updateSuscripcionEstado(int id, bool activa);
    bool removeSuscripcion(int id);

    bool addUser(const QString& username, const QString& password);
    bool login(const QString& username, const QString& password);
    bool userExists(const QString& username);
    void migrateUsers() {}

    double getGastoMes(int year, int month);
    int getTicketCountMes(int year, int month);
    int getSuscripcionesActivas();
    QVector<QPair<QString, double>> getGastosPorCategoria(int year, int month);
    QVector<QPair<QString, double>> getGastosPorSemana(int year, int month);

    enum EstadoRed { ESPERANDO, ENVIANDO_FOTO, SINCRONIZANDO, EXITO, ERROR_CONEXION };
    void analizarTicketRed(const QString& rutaImagen);
    void guardarTicketCompletoServidor(const QJsonObject& jsonCompleto);
    void registrarUsuarioRed(const QString& username, const QString& email, const QString& password);
    void guardarSuscripcionRed(const Suscripcion& s);
    void cambiarEstadoRed(EstadoRed nuevoEstado);

signals:
    void loginExitoso(int idUsuario, const QString& nombre);
    void loginFallido(const QString& mensaje);
    void notificacionesChanged();
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
    int     m_pendingUserId   = -1;
    QString m_pendingUsername;

    QVector<Notificacion> m_notificaciones;
    QString m_ultimoEmail;
};