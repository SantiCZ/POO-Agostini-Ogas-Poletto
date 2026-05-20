#include "datamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <QMap>

// ─── INCLUDES DE RED ──────────────────────────────────────────────
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QUrl>
#include <QFileInfo>
#include <QNetworkRequest>

DataManager::DataManager() : QObject(nullptr), estadoActual(ESPERANDO) {
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &DataManager::onRespuestaRecibida);
}

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
        m_users.clear();
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

    m_users.clear();
    QJsonArray usersArr = root["usuarios"].toArray();
    for (const QJsonValue& v : usersArr) {
        QJsonObject obj = v.toObject();
        User u;
        u.id = obj["id"].toInt();
        u.username = obj["username"].toString();
        u.passwordHash = obj["passwordHash"].toString();
        m_users.append(u);
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

    QJsonArray usersArr;
    for (const User& u : m_users) {
        QJsonObject obj;
        obj["id"] = u.id;
        obj["username"] = u.username;
        obj["passwordHash"] = u.passwordHash;
        usersArr.append(obj);
    }
    root["usuarios"] = usersArr;

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
        if (matchCat && matchSearch) result.append(t);
    }
    return result;
}

bool DataManager::addTicket(const Ticket& t) {
    // Verificamos si ya existe un ticket con el mismo ID para evitar duplicados
    for (const Ticket& existingT : m_tickets) {
        if (existingT.id == t.id) {
            qDebug() << "Intento de duplicado detectado, ignorando.";
            return false;
        }
    }

    Ticket newT = t;
    // Si el ID es 0, asignamos uno nuevo, si ya tiene ID, lo respetamos
    if (newT.id == 0) newT.id = nextTicketId();

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

QVector<Suscripcion> DataManager::getSuscripciones() const { return m_suscripciones; }

bool DataManager::addSuscripcion(const Suscripcion& s) {
    Suscripcion newS = s;
    newS.id = nextSubId();
    newS.fecha = QDate::currentDate();
    m_suscripciones.append(newS);
    return saveToFile();
}

bool DataManager::updateSuscripcionEstado(int id, bool activa) {
    for (Suscripcion& s : m_suscripciones) {
        if (s.id == id) { s.activa = activa; return saveToFile(); }
    }
    return false;
}

bool DataManager::removeSuscripcion(int id) {
    for (int i = 0; i < m_suscripciones.size(); ++i) {
        if (m_suscripciones[i].id == id) { m_suscripciones.remove(i); return saveToFile(); }
    }
    return false;
}

// ──────────────────────────────────────────────────────────────
// ESTADÍSTICAS (Las que faltaban)
// ──────────────────────────────────────────────────────────────

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

// ──────────────────────────────────────────────────────────────
// OTROS MÉTODOS Y RED
// ──────────────────────────────────────────────────────────────

QString DataManager::hashPassword(const QString& pwd) const {
    QByteArray hash = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

int DataManager::nextUserId() const {
    int max = 0;
    for (const User& u : m_users) if (u.id > max) max = u.id;
    return max + 1;
}

bool DataManager::addUser(const QString& username, const QString& password) {
    if (userExists(username)) return false;
    User u;
    u.id = nextUserId();
    u.username = username;
    u.passwordHash = hashPassword(password);
    m_users.append(u);
    return saveToFile();
}

bool DataManager::login(const QString& username, const QString& password) {
    QString hash = hashPassword(password);
    for (const User& u : m_users) {
        if (u.username == username && u.passwordHash == hash) return true;
    }
    return false;
}

bool DataManager::userExists(const QString& username) {
    for (const User& u : m_users) {
        if (u.username == username) return true;
    }
    return false;
}

void DataManager::migrateUsers() {
    if (m_users.isEmpty()) { addUser("alcancia", "1234"); }
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
        if (t.fecha.year() == year && t.fecha.month() == month) total += t.monto;
    return total;
}

int DataManager::getTicketCountMes(int year, int month) const {
    int count = 0;
    for (const Ticket& t : m_tickets)
        if (t.fecha.year() == year && t.fecha.month() == month) count++;
    return count;
}

int DataManager::getSuscripcionesActivas() const {
    int count = 0;
    for (const Suscripcion& s : m_suscripciones) if (s.activa) count++;
    return count;
}

void DataManager::cambiarEstadoRed(EstadoRed nuevoEstado) {
    estadoActual = nuevoEstado;
    emit estadoRedCambiado(estadoActual);
}

void DataManager::analizarTicketRed(const QString &rutaImagen) {
    cambiarEstadoRed(ENVIANDO_FOTO);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(rutaImagen).fileName() + "\""));

    QFile *file = new QFile(rutaImagen);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorDeRed("Error abriendo imagen.");
        delete multiPart; delete file;
        cambiarEstadoRed(ERROR_CONEXION);
        return;
    }
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(imagePart);

    QUrl url("http://161.97.92.143/api/v1/tickets/analizar");
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->post(request, multiPart);
    multiPart->setParent(reply);
}

void DataManager::guardarTicketCompletoServidor(const QJsonObject &jsonCompleto) {
    QUrl url("http://161.97.92.143/api/v1/tickets/guardar");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc(jsonCompleto);
    networkManager->post(request, doc.toJson());
}

void DataManager::registrarUsuarioRed(const QString &username, const QString &email, const QString &password) {
    cambiarEstadoRed(ESPERANDO);
    QUrl url("http://161.97.92.143/api/v1/usuarios/registro");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString claveEncriptada = hashPassword(password);
    QJsonObject jsonUser;
    jsonUser["nombre"] = username;
    jsonUser["email"] = email;
    jsonUser["clave_hash"] = claveEncriptada;
    QJsonDocument doc(jsonUser);
    networkManager->post(request, doc.toJson());
}

void DataManager::guardarSuscripcionRed(const Suscripcion &s) {
    QUrl url("http://161.97.92.143/api/v1/suscripciones/guardar");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["id_usuario"] = 1;
    json["nombre"] = s.nombreServicio;
    json["monto"] = s.monto;
    json["categoria"] = s.categoria;
    json["vencimiento"] = s.fechaVencimiento.toString(Qt::ISODate);
    json["alerta_dias"] = s.diasAviso;
    json["notas"] = s.descripcion.isEmpty() ? "Suscripción agregada desde UI" : s.descripcion;
    json["moneda"] = "ARS";
    json["frecuencia"] = "mensual";

    QJsonDocument doc(json);
    networkManager->post(request, doc.toJson());
}

void DataManager::onRespuestaRecibida(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorDeRed("Error de red: " + reply->errorString());
        cambiarEstadoRed(ERROR_CONEXION);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject()) { reply->deleteLater(); return; }

    QJsonObject root = jsonDoc.object();
    QString path = reply->url().path();

    if (path.contains("/analizar")) {
        QJsonObject gastoObj = root["gasto"].toObject();
        emit ticketProcesadoRed(gastoObj["comercio"].toString(), gastoObj["monto"].toDouble(), gastoObj["fecha_gasto"].toString(), gastoObj["categoria_sugerida"].toString(), root);
        cambiarEstadoRed(EXITO);
    } else if (path.contains("/guardar")) {
        emit ticketGuardadoServidor(root["status"].toString() == "ok", root["message"].toString());
    } else if (path.contains("/registro")) {
        emit usuarioRegistradoServidor(root["status"].toString() == "ok", root["message"].toString());
    } else if (path.contains("/suscripciones/guardar")) {
        emit suscripcionGuardadaServidor(root["status"].toString() == "ok", root["message"].toString());
    }
    reply->deleteLater();
}