#include "datamanager.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMap>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>

/*
 * datamanager.cpp
 * Implementa la capa de coordinacion entre UI, SQLite y servidor.
 * Aqui se concentran los flujos asincronicos de red y la conversion de datos
 * entre modelos C++, JSON y registros locales.
 *
 * Flujo general:
 * UI -> DataManager -> SQLite/VPS -> signals -> UI.
 */

// ─────────────────────────────────────────
// CONSTRUCTOR
// ─────────────────────────────────────────

DataManager::DataManager()
    : QObject(nullptr)
{
    // Todas las respuestas HTTP vuelven a onRespuestaRecibida para mantener
    // un unico punto de control del estado de red.
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &DataManager::onRespuestaRecibida);

    // DataManager abre la misma base local que usa el arranque. Asi la app
    // puede consultar cache offline y luego sincronizar contra el VPS.
    QString rutaDB = QFileInfo(__FILE__).absolutePath() + "/tasty_alcancia.db";
    m_db.conectar(rutaDB);
}

// ─────────────────────────────────────────
// SINGLETON
// ─────────────────────────────────────────

DataManager& DataManager::instance()
{
    static DataManager instance;
    return instance;
}

// ─────────────────────────────────────────
// HASH PASSWORD
// ─────────────────────────────────────────

QString DataManager::hashPassword(const QString& pwd) const
{
    return QString(QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5).toHex());
}

// ─────────────────────────────────────────
// CAMBIO ESTADO RED
// ─────────────────────────────────────────

void DataManager::cambiarEstadoRed(EstadoRed nuevoEstado)
{
    estadoActual = nuevoEstado;
    emit estadoRedCambiado(estadoActual);
}

// ─────────────────────────────────────────
// BASE DE DATOS
// ─────────────────────────────────────────

QSqlDatabase DataManager::getDB()
{
    return m_db.getDB();
}

// ─────────────────────────────────────────
// USUARIO ACTIVO ID
// ─────────────────────────────────────────

int DataManager::getUsuarioActivoId()
{
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT id_usuario_remoto
        FROM usuario_sesion
        WHERE sesion_activa = 1
        LIMIT 1
    )");
    if (query.exec() && query.next())
        return query.value(0).toInt();
    return -1;
}

// ─────────────────────────────────────────
// LOGIN REAL CONTRA EL VPS
// ─────────────────────────────────────────

void DataManager::loginRed(const QString& email, const QString& password)
{
    cambiarEstadoRed(SINCRONIZANDO);
    QUrl url("http://161.97.92.143/api/v1/usuarios/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject json;
    json["email"] = email;
    json["clave_hash"] = hashPassword(password);
    networkManager->post(request, QJsonDocument(json).toJson());
}

// ─────────────────────────────────────────
// SINCRONIZACION
// ─────────────────────────────────────────

void DataManager::sincronizarDesdeServidor(int id_usuario)
{
    cambiarEstadoRed(SINCRONIZANDO);
    QUrl url(QString("http://161.97.92.143/api/v1/usuarios/%1/sync").arg(id_usuario));
    QNetworkRequest request(url);
    networkManager->get(request);
}

void DataManager::sincronizarSuscripcionesLocales()
{
    // Recorre suscripciones con cambios locales y las envia al VPS.
    // Al terminar, el servidor queda como fuente comun para otros dispositivos.
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) {
        qDebug() << "No hay usuario activo para sincronizar.";
        emit syncSuscripcionesLocalesCompletada(false);
        return;
    }

    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT id_suscripcion_remota,
               nombre,
               monto,
               moneda,
               frecuencia,
               vencimiento,
               alerta,
               actividad,
               notas,
               accion_pendiente
        FROM suscripciones
        WHERE id_usuario_remoto = :uid
        AND (sincronizado = 0 OR accion_pendiente != 'ninguna')
    )");
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error buscando suscripciones pendientes:" << query.lastError().text();
        emit syncSuscripcionesLocalesCompletada(false);
        return;
    }

    QJsonArray subsArray;
    while (query.next()) {
        QJsonObject subJson;
        subJson["id_suscripcion_remota"] = query.value("id_suscripcion_remota").toInt();
        subJson["nombre"] = query.value("nombre").toString();
        subJson["monto"] = query.value("monto").toDouble();
        subJson["moneda"] = query.value("moneda").toString();
        subJson["frecuencia"] = query.value("frecuencia").toString();
        subJson["vencimiento"] = query.value("vencimiento").toString();
        subJson["alerta"] = query.value("alerta").toInt();
        subJson["actividad"] = query.value("actividad").toInt();
        subJson["notas"] = query.value("notas").toString();
        subJson["accion_pendiente"] = query.value("accion_pendiente").toString();
        subsArray.append(subJson);
    }

    if (subsArray.isEmpty()) {
        emit syncSuscripcionesLocalesCompletada(true);
        return;
    }

    cambiarEstadoRed(SINCRONIZANDO);
    QUrl url("http://161.97.92.143/api/v1/suscripciones/sync");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject jsonRaiz;
    jsonRaiz["id_usuario"] = usuarioId;
    jsonRaiz["suscripciones"] = subsArray;
    networkManager->post(request, QJsonDocument(jsonRaiz).toJson());
}

// ─────────────────────────────────────────
// VERIFICAR SI HAY CAMBIOS PENDIENTES (PRIVADO)
// ─────────────────────────────────────────

bool DataManager::haySuscripcionesPendientes()
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) return false;
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT COUNT(*)
        FROM suscripciones
        WHERE id_usuario_remoto = :uid
        AND (sincronizado = 0 OR accion_pendiente != 'ninguna')
    )");
    query.bindValue(":uid", usuarioId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// ─────────────────────────────────────────
// ESPERA SÍNCRONA PARA CIERRE DE APP (CORREGIDO)
// ─────────────────────────────────────────

bool DataManager::syncSuscripcionesLocalesAndWait(int timeoutMs)
{
    // SIGNAL/SLOT:
    // Se conecta temporalmente a syncSuscripcionesLocalesCompletada para salir
    // del EventLoop cuando termina la red. Esto adapta un flujo asincronico a
    // una espera controlada durante cierre/logout.
    // Qt usa red asincronica; para cierre de app se usa un EventLoop temporal
    // que espera la senal de finalizacion o corta por timeout.
    // Si no hay cambios pendientes, salimos inmediatamente
    if (!haySuscripcionesPendientes()) {
        return true;
    }

    QEventLoop loop;
    bool finished = false;
    bool success = false;
    connect(this, &DataManager::syncSuscripcionesLocalesCompletada,
            [&](bool exito) {
                success = exito;
                finished = true;
                loop.quit();
            });
    sincronizarSuscripcionesLocales();
    QTimer::singleShot(timeoutMs, &loop, [&]() {
        if (!finished) {
            finished = true;
            success = false;
            loop.quit();
        }
    });
    loop.exec();
    return success;
}

// ─────────────────────────────────────────
// RESPUESTAS RED
// ─────────────────────────────────────────

void DataManager::onRespuestaRecibida(QNetworkReply *reply)
{
    // SLOT:
    // Este metodo es slot de QNetworkAccessManager::finished. Procesa login,
    // sync, tickets y suscripciones segun la ruta del API.
    // Todas las rutas del API se discriminan por path para actualizar modelos,
    // base local y senales de UI segun la operacion que respondio.
    QByteArray responseData = reply->readAll();
    QString path = reply->url().path();

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "ERROR HTTP:" << httpStatus;
        qDebug() << "ERROR RED:" << reply->errorString();
        qDebug() << "RESPUESTA ERROR:" << responseData;

        // Caso especial: login con usuario o contraseña incorrectos
        if (path.contains("/login") && (httpStatus == 401 || httpStatus == 403))
        {
            emit loginFallido("Email o contraseña incorrectos.");
        }
        else
        {
            emit errorDeRed("No se pudo conectar con el servidor. Verificá tu conexión.");
        }

        cambiarEstadoRed(ERROR_CONEXION);
        reply->deleteLater();
        return;
    }

    // ─── LOGIN ───────────────────────────────────
    if (path.contains("/login")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        if (root["status"].toString() == "ok") {
            QJsonObject usuario = root["usuario"].toObject();
            int idUsuario = usuario["id_usuario"].toInt();
            QString nombre = usuario["nombre"].toString();

            // El login remoto tambien actualiza la sesion local. Esa sesion se
            // usa luego para filtrar SQLite y permitir operaciones offline.
            QSqlQuery query(m_db.getDB());
            query.prepare(R"(
                INSERT OR REPLACE INTO usuario_sesion
                (id_usuario_remoto, nombre, sesion_activa)
                VALUES (:id, :nombre, 1)
            )");
            query.bindValue(":id", idUsuario);
            query.bindValue(":nombre", nombre);
            if (!query.exec())
                qDebug() << "Error guardando sesion:" << query.lastError().text();

            m_pendingUserId   = idUsuario;
            m_pendingUsername = nombre;

            // Despues de autenticar, se baja el estado completo del usuario.
            // Cuando termina, esta misma funcion emite las senales de refresco.
            sincronizarDesdeServidor(idUsuario);
            emit loginExitoso(idUsuario, nombre);
        } else {
            emit loginFallido(root["message"].toString());
            cambiarEstadoRed(ERROR_CONEXION);
        }
    }
    // ─── SINCRONIZACIÓN DE SUSCRIPCIONES LOCALES ───
    else if (path.contains("/suscripciones/sync")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();

        if (root["status"].toString() == "ok") {
            // Una vez que el VPS acepta los cambios locales, se limpian marcas
            // de sincronizacion para no reenviar la misma suscripcion.
            QSqlQuery limpiar(m_db.getDB());
            limpiar.prepare(R"(
            UPDATE suscripciones
            SET sincronizado = 1,
                accion_pendiente = 'ninguna'
            WHERE sincronizado = 0
            OR accion_pendiente != 'ninguna'
        )");

            if (!limpiar.exec()) {
                qDebug() << "Error limpiando pendientes despues del sync:"
                         << limpiar.lastError().text();
            }

            int idUsuario = getUsuarioActivoId();
            if (idUsuario > 0) {
                // Se vuelve a bajar el estado remoto para que SQLite quede
                // alineado con el servidor despues del envio.
                sincronizarDesdeServidor(idUsuario);
            }

            emit syncSuscripcionesLocalesCompletada(true);
        }else {
            emit errorDeRed("Error sincronizando suscripciones: " + root["message"].toString());
            emit syncSuscripcionesLocalesCompletada(false);
            cambiarEstadoRed(ERROR_CONEXION);
        }
    }
    // ─── SYNC GENERAL ───────────────────────────
    else if (path.contains("/sync")) {
        if (m_db.sincronizarDesdeJson(responseData)) {
            // Al terminar la carga desde servidor, se aplican reglas locales
            // derivadas: renovar vencidas, generar avisos y refrescar UI.
            m_db.renovarSuscripcionesVencidas();
            m_db.generarNotificacionesVencimiento();
            cargarNotificacionesDesdeSQLite();
            emit sincronizacionCompletada();
            emit ticketsChanged();
            emit suscripcionesChanged();
        }
        cambiarEstadoRed(EXITO);
    }
    // ─── ANALIZAR IA ─────────────────────────────
    else if (path.contains("/analizar")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        QJsonObject gastoObj = root["gasto"].toObject();
        emit ticketProcesadoRed(gastoObj["comercio"].toString(),
                                gastoObj["monto"].toDouble(),
                                gastoObj["fecha_gasto"].toString(),
                                gastoObj["categoria_sugerida"].toString(),
                                root);
        cambiarEstadoRed(EXITO);
    }
    // ─── GUARDAR TICKET ─────────────────────────
    else if (path.contains("/tickets/guardar")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        emit ticketGuardadoServidor(root["status"].toString() == "ok", root["message"].toString());
        cambiarEstadoRed(EXITO);
    }
    // ─── GUARDAR SUSCRIPCION ─────────────────────
    else if (path.contains("/suscripciones/guardar")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();

        // Flujo correcto:
        // 1) Guardar nueva suscripción en VPS.
        // 2) Enviar pendientes locales: editar/eliminar.
        // 3) Recién después hacer sync general desde VPS.
        if (root["status"].toString() == "ok") {
            sincronizarSuscripcionesLocales();
        } else {
            emit errorDeRed("Error guardando suscripción: " + root["message"].toString());
            cambiarEstadoRed(ERROR_CONEXION);
        }
    }
    // ─── REGISTRO ────────────────────────────────
    else if (path.contains("/registro")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        emit usuarioRegistradoServidor(root["status"].toString() == "ok", root["message"].toString());
        cambiarEstadoRed(EXITO);
    }

    reply->deleteLater();
}

// ─────────────────────────────────────────
// ANALIZAR TICKET IA
// ─────────────────────────────────────────

void DataManager::analizarTicketRed(const QString& rutaImagen)
{
    cambiarEstadoRed(ENVIANDO_FOTO);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;

    QString mimeType = "image/jpeg";
    if (rutaImagen.endsWith(".pdf", Qt::CaseInsensitive)) {
        mimeType = "application/pdf";
    }

    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(rutaImagen).fileName() + "\""));

    QFile *file = new QFile(rutaImagen);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorDeRed("Error abriendo el archivo.");
        delete file;
        delete multiPart;
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

// ─────────────────────────────────────────
// GUARDAR TICKET SERVIDOR
// ─────────────────────────────────────────

void DataManager::guardarTicketCompletoServidor(const QJsonObject& jsonCompleto)
{
    cambiarEstadoRed(SINCRONIZANDO);
    QUrl url("http://161.97.92.143/api/v1/tickets/guardar");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkManager->post(request, QJsonDocument(jsonCompleto).toJson());
}

// ─────────────────────────────────────────
// REGISTRO USUARIO
// ─────────────────────────────────────────

void DataManager::registrarUsuarioRed(const QString& username, const QString& email, const QString& password)
{
    QUrl url("http://161.97.92.143/api/v1/usuarios/registro");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject json;
    json["nombre"] = username;
    json["email"] = email;
    json["clave_hash"] = hashPassword(password);
    networkManager->post(request, QJsonDocument(json).toJson());
}

// ─────────────────────────────────────────
// GUARDAR SUSCRIPCION SERVIDOR
// ─────────────────────────────────────────

void DataManager::guardarSuscripcionRed(const Suscripcion& s)
{
    cambiarEstadoRed(SINCRONIZANDO);
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) {
        qDebug() << "Usuario invalido";
        return;
    }

    QUrl url("http://161.97.92.143/api/v1/suscripciones/guardar");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["id_usuario"]   = usuarioId;
    json["nombre"]       = s.nombreServicio;
    json["monto"]        = s.monto;
    json["moneda"]       = "ARS";
    json["frecuencia"]   = "mensual";
    json["vencimiento"]  = s.fechaVencimiento.toString(Qt::ISODate);
    json["alerta"]       = s.diasAviso;
    json["actividad"]    = s.activa ? 1 : 0;
    json["notas"]        = "";

    networkManager->post(request, QJsonDocument(json).toJson());
}

// ─────────────────────────────────────────
// TICKETS
// ─────────────────────────────────────────

QVector<Ticket> DataManager::getTickets(const QString& categoriaFiltro, const QString& busqueda)
{
    QVector<Ticket> tickets;
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) return tickets;

    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT
            g.id_gasto,
            g.comercio,
            g.monto,
            g.fecha_gasto,
            c.nombre AS categoria,
            g.notas
        FROM gastos g
        LEFT JOIN categorias c ON g.id_categoria = c.id_categoria
        WHERE g.id_usuario = :uid
        ORDER BY g.fecha_gasto DESC
    )");
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error getTickets:" << query.lastError().text();
        return tickets;
    }

    while (query.next()) {
        Ticket t;
        t.id          = query.value("id_gasto").toInt();
        t.nombreLocal = query.value("comercio").toString();
        t.monto       = query.value("monto").toDouble();
        t.fecha       = QDate::fromString(query.value("fecha_gasto").toString(), Qt::ISODate);
        t.categoria   = query.value("categoria").toString();
        if (t.categoria.isEmpty()) {
            t.categoria = query.value("notas").toString();
        }
        t.procesadoPorIA = false;
        t.imagenPath  = "";

        bool coincideCategoria = categoriaFiltro.isEmpty()
                                 || categoriaFiltro == "Todas las categorías"
                                 || t.categoria.contains(categoriaFiltro, Qt::CaseInsensitive);
        bool coincideBusqueda = busqueda.isEmpty()
                                || t.nombreLocal.contains(busqueda, Qt::CaseInsensitive);

        if (coincideCategoria && coincideBusqueda)
            tickets.append(t);
    }
    return tickets;
}

bool DataManager::addTicket(const Ticket& t)
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) {
        qDebug() << "No hay usuario activo.";
        return false;
    }

    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        INSERT INTO gastos
        (id_usuario, comercio, monto, fecha_gasto, notas)
        VALUES (:uid, :comercio, :monto, :fecha, :notas)
    )");
    query.bindValue(":uid",      usuarioId);
    query.bindValue(":comercio", t.nombreLocal);
    query.bindValue(":monto",    t.monto);
    query.bindValue(":fecha",    t.fecha.toString(Qt::ISODate));
    query.bindValue(":notas",    t.categoria);

    if (!query.exec()) {
        qDebug() << "Error insertando ticket:" << query.lastError().text();
        return false;
    }

    // Primero se guarda localmente para que la UI responda rapido. Despues se
    // arma el JSON completo y se envia al servidor para sincronizar.
    QJsonObject jsonRaiz;
    jsonRaiz.insert("id_usuario", usuarioId);
    QJsonObject gastoObj;
    gastoObj.insert("comercio", t.nombreLocal);
    gastoObj.insert("monto", t.monto);
    gastoObj.insert("fecha_gasto", t.fecha.toString(Qt::ISODate));
    gastoObj.insert("categoria_sugerida", t.categoria);
    gastoObj.insert("notas", "Generado localmente");
    jsonRaiz.insert("gasto", gastoObj);
    QJsonObject compObj;
    if (t.imagenPath.isEmpty())
        compObj.insert("ruta_archivo", "carga_manual");
    else
        compObj.insert("ruta_archivo", t.imagenPath);
    compObj.insert("estado", "procesado");
    jsonRaiz.insert("comprobante", compObj);

    if (!t.items.isEmpty()) {
        QJsonArray itemsArray;
        for (const ItemTicket& item : t.items) {
            QJsonObject itemJson;
            itemJson.insert("descripcion", item.nombre);
            itemJson.insert("cantidad", item.cantidad);
            itemJson.insert("precio_unitario", item.precioUnitario);
            itemJson.insert("subtotal", item.cantidad * item.precioUnitario);
            itemsArray.append(itemJson);
        }
        jsonRaiz.insert("items_gasto", itemsArray);
    }

    guardarTicketCompletoServidor(jsonRaiz);
    emit ticketsChanged();
    return true;
}

bool DataManager::removeTicket(int id)
{
    int usuarioId = getUsuarioActivoId();
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        DELETE FROM gastos
        WHERE id_gasto = :id
        AND id_usuario = :uid
    )");
    query.bindValue(":id",  id);
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error eliminando ticket:" << query.lastError().text();
        return false;
    }
    emit ticketsChanged();
    return true;
}

// ─────────────────────────────────────────
// SUSCRIPCIONES
// ─────────────────────────────────────────

QVector<Suscripcion> DataManager::getSuscripciones()
{
    QVector<Suscripcion> subs;
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) return subs;

    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT
            id_suscripcion_local AS id_suscripcion,
            nombre,
            monto,
            vencimiento,
            alerta,
            actividad
        FROM suscripciones
        WHERE id_usuario_remoto = :uid
        AND actividad = 1
        ORDER BY vencimiento ASC
    )");
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error getSuscripciones:" << query.lastError().text();
        return subs;
    }

    while (query.next()) {
        Suscripcion s;
        s.id = query.value("id_suscripcion").toInt();
        s.nombreServicio = query.value("nombre").toString();
        s.monto = query.value("monto").toDouble();
        s.fechaVencimiento = QDate::fromString(query.value("vencimiento").toString(), Qt::ISODate);
        s.diasAviso = query.value("alerta").toInt();
        s.activa = query.value("actividad").toBool();
        s.iconoNombre = query.value("nombre").toString().toLower();
        subs.append(s);
    }
    return subs;
}

bool DataManager::addSuscripcion(const Suscripcion& s)
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) {
        qDebug() << "No hay usuario activo.";
        return false;
    }

    // Las suscripciones nuevas se envian al VPS; la respuesta remota vuelve a
    // marcar el registro local como sincronizado desde onRespuestaRecibida().
    guardarSuscripcionRed(s);
    return true;
}

bool DataManager::updateSuscripcionEstado(int id, bool activa)
{
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        UPDATE suscripciones
        SET actividad = :actividad
        WHERE id_suscripcion_local = :id
    )");
    query.bindValue(":actividad", activa ? 1 : 0);
    query.bindValue(":id",        id);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return false;
    }
    emit suscripcionesChanged();
    return true;
}

bool DataManager::updateSuscripcion(const Suscripcion &s)
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0) {
        qDebug() << "No hay usuario activo para editar suscripcion.";
        return false;
    }

    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        UPDATE suscripciones
        SET nombre = :nombre,
            monto = :monto,
            vencimiento = :vencimiento,
            alerta = :alerta,
            sincronizado = 0,
            accion_pendiente = 'editar'
        WHERE id_suscripcion_local = :id
        AND id_usuario_remoto = :uid
    )");
    query.bindValue(":nombre", s.nombreServicio);
    query.bindValue(":monto", s.monto);
    query.bindValue(":vencimiento", s.fechaVencimiento.toString(Qt::ISODate));
    query.bindValue(":alerta", s.diasAviso);
    query.bindValue(":id", s.id);
    query.bindValue(":uid", usuarioId);

    if (!query.exec()) {
        qDebug() << "Error updateSuscripcion:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No se encontro la suscripcion para editar.";
        return false;
    }

    emit suscripcionesChanged();
    return true;
}

bool DataManager::removeSuscripcion(int id)
{
    int usuarioId = getUsuarioActivoId();
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        UPDATE suscripciones
        SET actividad = 0,
            sincronizado = 0,
            accion_pendiente = 'eliminar'
        WHERE id_suscripcion_local = :id
        AND id_usuario_remoto = :uid
    )");
    query.bindValue(":id",  id);
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error eliminando suscripcion:" << query.lastError().text();
        return false;
    }
    emit suscripcionesChanged();
    return true;
}

int DataManager::getSuscripcionesActivas()
{
    int usuarioId = getUsuarioActivoId();
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT COUNT(*)
        FROM suscripciones
        WHERE id_usuario_remoto = :uid
        AND actividad = 1
    )");
    query.bindValue(":uid", usuarioId);
    if (query.exec() && query.next())
        return query.value(0).toInt();
    return 0;
}

void DataManager::renovarSuscripcionesVencidasLocales()
{
    m_db.renovarSuscripcionesVencidas();
    emit suscripcionesChanged();
}

// ─────────────────────────────────────────
// USUARIOS
// ─────────────────────────────────────────

bool DataManager::addUser(const QString& username, const QString& password)
{
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        UPDATE usuario_sesion
        SET password_hash = :password,
            nombre        = :username
        WHERE sesion_activa = 1
    )");
    query.bindValue(":username", username);
    query.bindValue(":password", hashPassword(password));
    if (!query.exec()) {
        qDebug() << "Error guardando password:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DataManager::login(const QString& username, const QString& password)
{
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT nombre, password_hash
        FROM usuario_sesion
        WHERE sesion_activa = 1
        LIMIT 1
    )");
    if (!query.exec() || !query.next()) {
        qDebug() << "No hay sesion activa en sqlite (modo offline).";
        return false;
    }
    QString hashGuardado = query.value("password_hash").toString();
    if (hashGuardado.isEmpty())
        return true;
    return hashGuardado == hashPassword(password);
}

bool DataManager::userExists(const QString& username)
{
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT nombre
        FROM usuario_sesion
        WHERE sesion_activa = 1
    )");
    if (!query.exec() || !query.next())
        return false;
    return query.value("nombre").toString().compare(username, Qt::CaseInsensitive) == 0;
}

QString DataManager::getUltimoEmail() const
{
    QSettings settings("AlcancIA", "Login");
    return settings.value("ultimoEmail").toString();
}

void DataManager::setUltimoEmail(const QString &email)
{
    QSettings settings("AlcancIA", "Login");
    settings.setValue("ultimoEmail", email);
}

// ─────────────────────────────────────────
// ESTADISTICAS
// ─────────────────────────────────────────

double DataManager::getGastoMes(int year, int month)
{
    int usuarioId = getUsuarioActivoId();
    QString monthStr = QString("%1").arg(month, 2, 10, QChar('0'));
    QString sql = QString(
                      "SELECT COALESCE(SUM(monto), 0) FROM gastos "
                      "WHERE id_usuario = %1 "
                      "AND strftime('%%Y', fecha_gasto) = '%2' "
                      "AND strftime('%%m', fecha_gasto) = '%3'"
                      ).arg(usuarioId).arg(year).arg(monthStr);
    QSqlQuery query(m_db.getDB());
    query.exec(sql);
    if (query.next())
        return query.value(0).toDouble();
    return 0.0;
}

int DataManager::getTicketCountMes(int year, int month)
{
    int usuarioId = getUsuarioActivoId();
    QString monthStr = QString("%1").arg(month, 2, 10, QChar('0'));
    QString sql = QString(
                      "SELECT COUNT(*) FROM gastos "
                      "WHERE id_usuario = %1 "
                      "AND strftime('%%Y', fecha_gasto) = '%2' "
                      "AND strftime('%%m', fecha_gasto) = '%3'"
                      ).arg(usuarioId).arg(year).arg(monthStr);
    QSqlQuery query(m_db.getDB());
    query.exec(sql);
    if (query.next())
        return query.value(0).toInt();
    return 0;
}

QVector<QPair<QString,double>> DataManager::getGastosPorCategoria(int year, int month)
{
    QVector<QPair<QString,double>> resultado;
    int usuarioId = getUsuarioActivoId();
    QString monthStr = QString("%1").arg(month, 2, 10, QChar('0'));
    QString sql = QString(R"(
        SELECT c.nombre AS categoria, SUM(g.monto) AS total
        FROM gastos g
        LEFT JOIN categorias c ON g.id_categoria = c.id_categoria
        WHERE g.id_usuario = %1
        AND strftime('%%Y', g.fecha_gasto) = '%2'
        AND strftime('%%m', g.fecha_gasto) = '%3'
        GROUP BY c.nombre
        ORDER BY total DESC
    )").arg(usuarioId).arg(year).arg(monthStr);
    QSqlQuery query(m_db.getDB());
    if (!query.exec(sql)) {
        qDebug() << "Error getGastosPorCategoria:" << query.lastError().text();
        return resultado;
    }
    while (query.next())
        resultado.append(qMakePair(query.value("categoria").toString(), query.value("total").toDouble()));
    return resultado;
}

QVector<QPair<QString,double>> DataManager::getGastosPorSemana(int year, int month)
{
    QVector<QPair<QString,double>> resultado;
    int usuarioId = getUsuarioActivoId();
    QString monthStr = QString("%1").arg(month, 2, 10, QChar('0'));
    QString sql = QString(R"(
        SELECT
            ((CAST(strftime('%%d', fecha_gasto) AS INTEGER) - 1) / 7) + 1 AS semana,
            SUM(monto) AS total
        FROM gastos
        WHERE id_usuario = %1
        AND strftime('%%Y', fecha_gasto) = '%2'
        AND strftime('%%m', fecha_gasto) = '%3'
        GROUP BY semana
        ORDER BY semana ASC
    )").arg(usuarioId).arg(year).arg(monthStr);
    QSqlQuery query(m_db.getDB());
    if (!query.exec(sql)) {
        qDebug() << "Error getGastosPorSemana:" << query.lastError().text();
        return resultado;
    }
    while (query.next())
        resultado.append(qMakePair(QString("Semana %1").arg(query.value("semana").toInt()), query.value("total").toDouble()));
    return resultado;
}


// ─────────────────────────────────────────
// NOTIFICACIONES
// ─────────────────────────────────────────

QVector<Notificacion> DataManager::getNotificaciones() const
{
    return m_notificaciones;
}

void DataManager::agregarNotificacion(const Notificacion &n)
{
    m_notificaciones.push_back(n);
    emit notificacionesChanged();
}

void DataManager::cargarNotificacionesDesdeSQLite()
{
    // Este metodo reconstruye el cache en memoria desde SQLite. Se llama luego
    // de sincronizar para que la campanita y el panel muestren datos actuales.
    m_notificaciones.clear();
    int usuarioId = getUsuarioActivoId();
    QSqlQuery query(m_db.getDB());
    query.prepare(R"(
        SELECT mensaje, fecha_creacion
        FROM notificaciones
        WHERE id_usuario = :uid
        AND leida = 0
        ORDER BY fecha_creacion DESC
    )");
    query.bindValue(":uid", usuarioId);
    if (!query.exec()) {
        qDebug() << "Error cargando notificaciones:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        Notificacion n;
        n.mensaje = query.value("mensaje").toString();
        n.fecha = QDate::fromString(query.value("fecha_creacion").toString().left(10), Qt::ISODate);
        m_notificaciones.append(n);
    }
    emit notificacionesChanged();
}


