#pragma once
#include <QObject>
#include <QVector>
#include <QJsonObject>
#include "models.h"

// ─── LIBRERÍAS DE RED ─────────────────────────────────────────────
#include <QNetworkAccessManager>
#include <QNetworkReply>

class DataManager : public QObject {
    Q_OBJECT
public:
    // ─── ESTADOS DE RED ───────────────────────────────────────────
    enum EstadoRed { ESPERANDO, ENVIANDO_FOTO, EXITO, ERROR_CONEXION };
    Q_ENUM(EstadoRed)

    static DataManager& instance();

    bool loadFromFile();
    bool saveToFile();

    // Tickets
    QVector<Ticket> getTickets(const QString& categoriaFiltro = "", const QString& busqueda = "") const;
    bool addTicket(const Ticket& t);
    bool removeTicket(int id);

    // Suscripciones
    QVector<Suscripcion> getSuscripciones() const;
    bool addSuscripcion(const Suscripcion& s);
    bool updateSuscripcionEstado(int id, bool activa);
    bool removeSuscripcion(int id);

    // Usuarios
    bool addUser(const QString& username, const QString& password);
    bool login(const QString& username, const QString& password);
    bool userExists(const QString& username);
    void migrateUsers();

    // Estadísticas
    double getGastoMes(int year, int month) const;
    int getTicketCountMes(int year, int month) const;
    int getSuscripcionesActivas() const;
    QVector<QPair<QString, double>> getGastosPorCategoria(int year, int month) const;
    QVector<QPair<QString, double>> getGastosPorSemana(int year, int month) const;

    // ─── MÉTODOS PÚBLICOS DE RED ──────────────────────────────────
    void analizarTicketRed(const QString &rutaImagen);
    void guardarTicketCompletoServidor(const QJsonObject &jsonCompleto);
    void registrarUsuarioRed(const QString &username, const QString &email, const QString &password);
    void guardarSuscripcionRed(const Suscripcion &s); // NUEVO: Enviar suscripción al VPS

signals:
    // ─── SEÑALES DE RED ───────────────────────────────────────────
    void ticketProcesadoRed(const QString &comercio, double monto, const QString &fecha, const QString &categoria, const QJsonObject &jsonCompleto);
    void errorDeRed(const QString &mensaje);
    void estadoRedCambiado(DataManager::EstadoRed nuevoEstado);
    void ticketGuardadoServidor(bool exito, const QString &mensaje);
    void usuarioRegistradoServidor(bool exito, const QString &mensaje);
    void suscripcionGuardadaServidor(bool exito, const QString &mensaje); // NUEVO: Respuesta de suscripción

private slots:
    // ─── SLOT DE RESPUESTA DE RED ─────────────────────────────────
    void onRespuestaRecibida(QNetworkReply *reply);

private:
    DataManager();

    QString dataFilePath() const;
    QString hashPassword(const QString& pwd) const;
    int nextTicketId() const;
    int nextSubId() const;
    int nextUserId() const;

    QVector<Ticket> m_tickets;
    QVector<Suscripcion> m_suscripciones;
    QVector<User> m_users;

    // ─── VARIABLES DE RED ─────────────────────────────────────────
    QNetworkAccessManager *networkManager;
    EstadoRed estadoActual;
    void cambiarEstadoRed(EstadoRed nuevoEstado);
};