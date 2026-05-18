#pragma once
#include <QObject>
#include <QVector>
#include "models.h"

class DataManager : public QObject {
    Q_OBJECT
public:
    static DataManager& instance();

    bool loadFromFile();
    bool saveToFile();

    // Tickets con filtros
    QVector<Ticket> getTickets(const QString& categoriaFiltro = "", const QString& busqueda = "") const;
    bool addTicket(const Ticket& t);
    bool removeTicket(int id);

    // Suscripciones
    QVector<Suscripcion> getSuscripciones() const;
    bool addSuscripcion(const Suscripcion& s);
    bool updateSuscripcionEstado(int id, bool activa);
    bool removeSuscripcion(int id);

    // Estadísticas
    double getGastoMes(int year, int month) const;
    int getTicketCountMes(int year, int month) const;
    int getSuscripcionesActivas() const;
    QVector<QPair<QString, double>> getGastosPorCategoria(int year, int month) const;
    QVector<QPair<QString, double>> getGastosPorSemana(int year, int month) const;

private:
    DataManager() = default;
    QString dataFilePath() const;
    QVector<Ticket> m_tickets;
    QVector<Suscripcion> m_suscripciones;
    int nextTicketId() const;
    int nextSubId() const;
};