#include "datamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DataManager& DataManager::instance() {
    static DataManager inst;
    return inst;
}

QString DataManager::dataFilePath() const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");
    return path + "/alcancia_data.json";
}

bool DataManager::loadFromFile() {
    QFile file(dataFilePath());
    if (!file.exists()) {
        m_tickets.clear();
        m_suscripciones.clear();
        return saveToFile();
    }
    if (!file.open(QIODevice::ReadOnly)) return false;
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) return false;

    QJsonObject root = doc.object();
    m_tickets.clear();
    QJsonArray ticketsArr = root["tickets"].toArray();
    for (const QJsonValue& v : ticketsArr) {
        QJsonObject obj = v.toObject();
        Ticket t;
        t.id = obj["id"].toInt();
        t.nombreLocal = obj["nombreLocal"].toString();
        t.monto = obj["monto"].toDouble();
        t.fecha = QDate::fromString(obj["fecha"].toString(), Qt::ISODate);
        t.categoria = obj["categoria"].toString();
        t.descripcion = obj["descripcion"].toString();
        t.imagenPath = obj["imagenPath"].toString();
        t.procesadoPorIA = obj["procesadoPorIA"].toBool();
        m_tickets.append(t);
    }
    m_suscripciones.clear();
    QJsonArray subsArr = root["suscripciones"].toArray();
    for (const QJsonValue& v : subsArr) {
        QJsonObject obj = v.toObject();
        Suscripcion s;
        s.id = obj["id"].toInt();
        s.nombreServicio = obj["nombreServicio"].toString();
        s.monto = obj["monto"].toDouble();
        s.fecha = QDate::fromString(obj["fecha"].toString(), Qt::ISODate);
        s.categoria = obj["categoria"].toString();
        s.descripcion = obj["descripcion"].toString();
        s.fechaVencimiento = QDate::fromString(obj["fechaVencimiento"].toString(), Qt::ISODate);
        s.diasAviso = obj["diasAviso"].toInt();
        s.activa = obj["activa"].toBool();
        s.iconoNombre = obj["iconoNombre"].toString();
        m_suscripciones.append(s);
    }
    return true;
}

bool DataManager::saveToFile() {
    QJsonObject root;
    QJsonArray ticketsArr;
    for (const Ticket& t : m_tickets) {
        QJsonObject obj;
        obj["id"] = t.id;
        obj["nombreLocal"] = t.nombreLocal;
        obj["monto"] = t.monto;
        obj["fecha"] = t.fecha.toString(Qt::ISODate);
        obj["categoria"] = t.categoria;
        obj["descripcion"] = t.descripcion;
        obj["imagenPath"] = t.imagenPath;
        obj["procesadoPorIA"] = t.procesadoPorIA;
        ticketsArr.append(obj);
    }
    root["tickets"] = ticketsArr;

    QJsonArray subsArr;
    for (const Suscripcion& s : m_suscripciones) {
        QJsonObject obj;
        obj["id"] = s.id;
        obj["nombreServicio"] = s.nombreServicio;
        obj["monto"] = s.monto;
        obj["fecha"] = s.fecha.toString(Qt::ISODate);
        obj["categoria"] = s.categoria;
        obj["descripcion"] = s.descripcion;
        obj["fechaVencimiento"] = s.fechaVencimiento.toString(Qt::ISODate);
        obj["diasAviso"] = s.diasAviso;
        obj["activa"] = s.activa;
        obj["iconoNombre"] = s.iconoNombre;
        subsArr.append(obj);
    }
    root["suscripciones"] = subsArr;

    QFile file(dataFilePath());
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson());
    return true;
}

QVector<Ticket> DataManager::getTickets(const QString& categoriaFiltro, const QString& busqueda) const {
    QVector<Ticket> result;
    for (const Ticket& t : m_tickets) {
        bool matchCat = categoriaFiltro.isEmpty() || categoriaFiltro == "Todas las categorías" || t.categoria == categoriaFiltro;
        bool matchSearch = busqueda.isEmpty() || t.nombreLocal.contains(busqueda, Qt::CaseInsensitive);
        if (matchCat && matchSearch)
            result.append(t);
    }
    return result;
}

bool DataManager::addTicket(const Ticket& t) {
    Ticket newT = t;
    newT.id = nextTicketId();
    m_tickets.append(newT);
    return saveToFile();
}

bool DataManager::removeTicket(int id) {
    for (int i = 0; i < m_tickets.size(); ++i) {
        if (m_tickets[i].id == id) {
            m_tickets.remove(i);
            return saveToFile();
        }
    }
    return false;
}

QVector<Suscripcion> DataManager::getSuscripciones() const {
    return m_suscripciones;
}

bool DataManager::addSuscripcion(const Suscripcion& s) {
    Suscripcion newS = s;
    newS.id = nextSubId();
    newS.fecha = QDate::currentDate();
    m_suscripciones.append(newS);
    return saveToFile();
}

bool DataManager::updateSuscripcionEstado(int id, bool activa) {
    for (Suscripcion& s : m_suscripciones) {
        if (s.id == id) {
            s.activa = activa;
            return saveToFile();
        }
    }
    return false;
}

bool DataManager::removeSuscripcion(int id) {
    for (int i = 0; i < m_suscripciones.size(); ++i) {
        if (m_suscripciones[i].id == id) {
            m_suscripciones.remove(i);
            return saveToFile();
        }
    }
    return false;
}

int DataManager::nextTicketId() const {
    int max = 0;
    for (const Ticket& t : m_tickets) if (t.id > max) max = t.id;
    return max + 1;
}

int DataManager::nextSubId() const {
    int max = 0;
    for (const Suscripcion& s : m_suscripciones) if (s.id > max) max = s.id;
    return max + 1;
}

double DataManager::getGastoMes(int year, int month) const {
    double total = 0;
    for (const Ticket& t : m_tickets)
        if (t.fecha.year() == year && t.fecha.month() == month)
            total += t.monto;
    return total;
}

int DataManager::getTicketCountMes(int year, int month) const {
    int count = 0;
    for (const Ticket& t : m_tickets)
        if (t.fecha.year() == year && t.fecha.month() == month)
            count++;
    return count;
}

int DataManager::getSuscripcionesActivas() const {
    int count = 0;
    for (const Suscripcion& s : m_suscripciones)
        if (s.activa) count++;
    return count;
}

QVector<QPair<QString, double>> DataManager::getGastosPorCategoria(int year, int month) const {
    QMap<QString, double> map;
    for (const Ticket& t : m_tickets) {
        if (t.fecha.year() == year && t.fecha.month() == month) {
            map[t.categoria] += t.monto;
        }
    }
    QVector<QPair<QString, double>> res;
    for (auto it = map.begin(); it != map.end(); ++it)
        res.append({it.key(), it.value()});
    return res;
}

QVector<QPair<QString, double>> DataManager::getGastosPorSemana(int year, int month) const {
    QVector<double> semanas(5, 0.0);
    for (const Ticket& t : m_tickets) {
        if (t.fecha.year() == year && t.fecha.month() == month) {
            int week = (t.fecha.day() - 1) / 7 + 1;
            if (week >= 1 && week <= 4) semanas[week] += t.monto;
        }
    }
    QVector<QPair<QString, double>> res;
    for (int i = 1; i <= 4; ++i)
        res.append({QString("Sem %1").arg(i), semanas[i]});
    return res;
}