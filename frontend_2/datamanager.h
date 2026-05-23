#pragma once
#include <QObject>
#include <QVector>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonObject>
#include "models.h"
// Incluimos admindb.h para que DataManager pueda gestionar SQLite
#include "admindb.h"

class DataManager : public QObject {
    Q_OBJECT
public:
    static DataManager& instance();

    // NUEVO: Método para forzar la sincronización completa desde el VPS al SQLite local
    // Esto reemplaza la carga desde archivos JSON
    void sincronizarDesdeServidor(int id_usuario);

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

    // --- FUNCIONES DE RED ---
    enum EstadoRed { ESPERANDO, ENVIANDO_FOTO, SINCRONIZANDO, EXITO, ERROR_CONEXION };

    void analizarTicketRed(const QString &rutaImagen);
    void guardarTicketCompletoServidor(const QJsonObject &jsonCompleto);
    void registrarUsuarioRed(const QString &username, const QString &email, const QString &password);
    void guardarSuscripcionRed(const Suscripcion &s);
    void cambiarEstadoRed(EstadoRed nuevoEstado);

signals:
    // --- SEÑALES ---
    void estadoRedCambiado(DataManager::EstadoRed estado);
    void errorDeRed(const QString &mensaje);
    void ticketProcesadoRed(const QString &comercio, double monto, const QString &fecha, const QString &categoria, const QJsonObject &jsonCompleto);
    void ticketGuardadoServidor(bool exito, const QString &mensaje);
    void usuarioRegistradoServidor(bool exito, const QString &mensaje);
    void suscripcionGuardadaServidor(bool exito, const QString &mensaje);
    // NUEVO: Avisa al UI cuando el SQLite local está listo tras la sincronización
    void sincronizacionCompletada();

private slots:
    void onRespuestaRecibida(QNetworkReply *reply);

private:
    DataManager(); // Singleton

    QString hashPassword(const QString& pwd) const;

    QNetworkAccessManager *networkManager;
    EstadoRed estadoActual;
};