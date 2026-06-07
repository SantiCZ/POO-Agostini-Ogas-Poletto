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

/*
 * DataManager
 * Responsabilidad de clase:
 * Fachada principal de datos de la aplicacion.
 * Centraliza el acceso a SQLite, las peticiones HTTP al VPS y la emision de
 * senales para que las pantallas se actualicen sin acoplarse a la base.
 *
 * Herencia:
 * Hereda de QObject para poder usar el sistema de SIGNAL/SLOT de Qt.
 *
 * Clases abstractas:
 * No es una clase abstracta porque se instancia como Singleton y ofrece una
 * implementacion concreta de datos, red y sincronizacion.
 */
class DataManager : public QObject {
    Q_OBJECT
public:
    // Singleton: todas las pantallas usan la misma instancia y el mismo estado.
    static DataManager& instance();

    // Estado observable de red para informar a la interfaz.
    enum EstadoRed { ESPERANDO, ENVIANDO_FOTO, SINCRONIZANDO, EXITO, ERROR_CONEXION };

    // Utilidades basicas de estado y base local.
    QSqlDatabase getDB();
    void cambiarEstadoRed(EstadoRed nuevoEstado);

    // Sesion y sincronizacion general con el servidor remoto.
    int getUsuarioActivoId();
    void loginRed(const QString& email, const QString& password);
    void sincronizarDesdeServidor(int id_usuario);
    void sincronizarSuscripcionesLocales();

    // Espera una sincronizacion antes de cerrar sesion o cerrar la app.
    bool syncSuscripcionesLocalesAndWait(int timeoutMs = 10000);   // NUEVO

    void analizarTicketRed(const QString& rutaImagen);
    void guardarTicketCompletoServidor(const QJsonObject& jsonCompleto);
    void registrarUsuarioRed(const QString& username, const QString& email, const QString& password);
    void guardarSuscripcionRed(const Suscripcion& s);

    // CRUD de tickets/gastos.
    QVector<Ticket> getTickets(const QString& categoriaFiltro = "", const QString& busqueda = "");
    bool addTicket(const Ticket& t);
    bool removeTicket(int id);

    // CRUD de suscripciones.
    QVector<Suscripcion> getSuscripciones();
    bool addSuscripcion(const Suscripcion& s);
    bool updateSuscripcionEstado(int id, bool activa);
    bool updateSuscripcion(const Suscripcion &s);
    bool removeSuscripcion(int id);
    int getSuscripcionesActivas();
    void renovarSuscripcionesVencidasLocales();

    // Usuarios locales: permiten login offline cuando ya existe una sesion.
    bool addUser(const QString& username, const QString& password);
    bool login(const QString& username, const QString& password);
    bool userExists(const QString& username);
    QString getUltimoEmail() const;
    void setUltimoEmail(const QString &email);
    void migrateUsers() {}

    // Consultas agregadas usadas por DashboardPage y ReportsPage.
    double getGastoMes(int year, int month);
    int getTicketCountMes(int year, int month);
    QVector<QPair<QString, double>> getGastosPorCategoria(int year, int month);
    QVector<QPair<QString, double>> getGastosPorSemana(int year, int month);

    // Notificaciones locales generadas por vencimientos de suscripciones.
    QVector<Notificacion> getNotificaciones() const;
    void agregarNotificacion(const Notificacion &n);
    void cargarNotificacionesDesdeSQLite();

signals:
    /*
     * SIGNAL:
     * Estas senales notifican a la UI cuando termina una accion asincronica
     * o cambia un conjunto de datos. Evitan que las pantallas consulten en
     * bucle o dependan directamente de los detalles de red/SQLite.
     */
    // Senales de autenticacion y registro.
    void loginExitoso(int idUsuario, const QString& nombre);
    void loginFallido(const QString& mensaje);

    // Senales para refrescar partes especificas de la interfaz.
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

    // Senal usada por la espera sincrona al cerrar sesion o aplicacion.
    void syncSuscripcionesLocalesCompletada(bool exito);

private slots:
    /*
     * SLOT:
     * Este slot recibe todas las respuestas HTTP. Qt lo ejecuta cuando
     * QNetworkAccessManager emite finished(), permitiendo procesar red de
     * forma asincronica sin bloquear la interfaz.
     */
    // Punto unico donde se procesan las respuestas HTTP del VPS.
    void onRespuestaRecibida(QNetworkReply* reply);

private:
    // Detecta registros locales con cambios pendientes de subir.
    bool haySuscripcionesPendientes();

    // Constructor privado para reforzar el patron Singleton.
    DataManager();
    QString hashPassword(const QString& pwd) const;

    // Atributo importante: cliente HTTP reutilizado para login,
    // sincronizacion, tickets e IA. Vive como miembro para mantenerlo durante
    // toda la vida del Singleton.
    QNetworkAccessManager* networkManager;

    // Atributo importante: estado visible de red que informa a la UI.
    EstadoRed estadoActual = ESPERANDO;

    // Atributo importante: administrador SQLite local.
    adminDB  m_db;

    // Datos temporales usados entre login remoto y sincronizacion.
    QString  m_rutaDB;
    int     m_pendingUserId   = -1;
    QString m_pendingUsername;

    // Cache local de notificaciones para no recalcular la UI siempre.
    QVector<Notificacion> m_notificaciones;
    QString m_ultimoEmail;
};
