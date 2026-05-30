// ============================
// datamanager.cpp
// ============================

#include "datamanager.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
#include <QMap>
#include <QSettings>// añado para recordar usuario


#include <QFile>
#include <QFileInfo>

#include <QSqlQuery>
#include <QSqlError>

#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QUrl>

// ─────────────────────────────────────────
// CONSTRUCTOR
// ─────────────────────────────────────────

DataManager::DataManager()
    : QObject(nullptr)
{
    networkManager =
        new QNetworkAccessManager(this);

    connect(
        networkManager,
        &QNetworkAccessManager::finished,
        this,
        &DataManager::onRespuestaRecibida
        );

    QString rutaDB =
        QFileInfo(__FILE__).absolutePath()
        + "/tasty_alcancia.db";

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

QString DataManager::hashPassword(
    const QString& pwd
    ) const
{
    return QString(
        QCryptographicHash::hash(
            pwd.toUtf8(),
            QCryptographicHash::Md5
            ).toHex()
        );
}

// ─────────────────────────────────────────
// CAMBIO ESTADO RED
// ─────────────────────────────────────────

void DataManager::cambiarEstadoRed(
    EstadoRed nuevoEstado
    )
{
    estadoActual = nuevoEstado;
    emit estadoRedCambiado(estadoActual);
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

void DataManager::loginRed(
    const QString& email,
    const QString& password
    )
{
    // el login real se valida en el vps, no en sqlite.
    // sqlite solo se usa como cache offline despues de un login exitoso.
    cambiarEstadoRed(SINCRONIZANDO);

    QUrl url("http://161.97.92.143/api/v1/usuarios/login");
    QNetworkRequest request(url);
    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );

    QJsonObject json;
    json["email"]     = email;
    json["clave_hash"] = hashPassword(password);

    networkManager->post(request, QJsonDocument(json).toJson());
}

// ─────────────────────────────────────────
// SINCRONIZACION
// ─────────────────────────────────────────

void DataManager::sincronizarDesdeServidor(
    int id_usuario
    )
{
    cambiarEstadoRed(SINCRONIZANDO);

    QUrl url(
        QString(
            "http://161.97.92.143/api/v1/usuarios/%1/sync"
            ).arg(id_usuario)
        );

    QNetworkRequest request(url);
    networkManager->get(request);
}

// ─────────────────────────────────────────
// RESPUESTAS RED
// ─────────────────────────────────────────

void DataManager::onRespuestaRecibida(
    QNetworkReply *reply
    )
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorDeRed(
            "Error de red: " + reply->errorString()
            );
        cambiarEstadoRed(ERROR_CONEXION);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QString path = reply->url().path();
    qDebug() << "URL:" << path;
    qDebug() << "RESPUESTA:";
    qDebug() << responseData;

    qDebug() << "JSON VPS:";
    qDebug().noquote() << responseData;

    // ─── LOGIN ───────────────────────────────────
    if (path.contains("/login"))
    {
        QJsonObject root =
            QJsonDocument::fromJson(responseData).object();

        if (root["status"].toString() == "ok")
        {
            QJsonObject usuario =
                root["usuario"].toObject();

            int idUsuario =
                usuario["id_usuario"].toInt();

            QString nombre =
                usuario["nombre"].toString();

            qDebug() << "Login VPS OK. usuario:" << nombre
                     << "id:" << idUsuario;

            // guardar sesion local
            QSqlQuery query(m_db.getDB());

            query.prepare(R"(
        INSERT OR REPLACE INTO usuario_sesion
        (
            id_usuario_remoto,
            nombre,
            sesion_activa
        )
        VALUES
        (
            :id,
            :nombre,
            1
        )
    )");

            query.bindValue(":id", idUsuario);
            query.bindValue(":nombre", nombre);

            if (!query.exec())
            {
                qDebug() << "Error guardando sesion:"
                         << query.lastError().text();
            }

            m_pendingUserId   = idUsuario;
            m_pendingUsername = nombre;

            sincronizarDesdeServidor(idUsuario);

            emit loginExitoso(idUsuario, nombre);
        }
        else
        {
            emit loginFallido(root["message"].toString());
            cambiarEstadoRed(ERROR_CONEXION);
        }
    }

    // ─── SYNC ────────────────────────────────────
    else if (path.contains("/sync"))
    {
        if (m_db.sincronizarDesdeJson(responseData))
        {
            // NUEVO: revisa las suscripciones sincronizadas y genera avisos de vencimiento
            m_db.generarNotificacionesVencimiento();

            qDebug() << "Sincronizacion OK";
            emit sincronizacionCompletada();
            emit ticketsChanged();
            emit suscripcionesChanged();
        }
        cambiarEstadoRed(EXITO);
    }

    // ─── ANALIZAR IA ─────────────────────────────
    else if (path.contains("/analizar"))
    {
        QJsonObject root =
            QJsonDocument::fromJson(responseData).object();
        QJsonObject gastoObj = root["gasto"].toObject();
        emit ticketProcesadoRed(
            gastoObj["comercio"].toString(),
            gastoObj["monto"].toDouble(),
            gastoObj["fecha_gasto"].toString(),
            gastoObj["categoria_sugerida"].toString(),
            root
            );
        cambiarEstadoRed(EXITO);
    }

    // ─── GUARDAR TICKET ──────────────────────────
    else if (path.contains("/tickets/guardar"))
    {
        QJsonObject root =
            QJsonDocument::fromJson(responseData).object();
        emit ticketGuardadoServidor(
            root["status"].toString() == "ok",
            root["message"].toString()
            );
        cambiarEstadoRed(EXITO);
    }

    // ─── GUARDAR SUSCRIPCION ─────────────────────
    else if (path.contains("/suscripciones/guardar"))
    {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();

        qDebug() << "RESPUESTA SUSCRIPCION:";
        qDebug() << root;

        // --- CORRECCIÓN: Sincronizar después de guardar ---
        int idUsuario = getUsuarioActivoId();
        if (idUsuario > 0) {
            sincronizarDesdeServidor(idUsuario);
        }
        // --------------------------------------------------

        cambiarEstadoRed(EXITO);
    }

    // ─── REGISTRO ────────────────────────────────
    else if (path.contains("/registro"))
    {
        QJsonObject root =
            QJsonDocument::fromJson(responseData).object();
        emit usuarioRegistradoServidor(
            root["status"].toString() == "ok",
            root["message"].toString()
            );
        cambiarEstadoRed(EXITO);
    }

    reply->deleteLater();
}

// ─────────────────────────────────────────
// ANALIZAR TICKET IA
// ─────────────────────────────────────────

void DataManager::analizarTicketRed(
    const QString& rutaImagen
    )
{
    cambiarEstadoRed(ENVIANDO_FOTO);

    QHttpMultiPart *multiPart =
        new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QVariant("image/jpeg")
        );
    imagePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(
            "form-data; name=\"file\"; filename=\""
            + QFileInfo(rutaImagen).fileName() + "\""
            )
        );

    QFile *file = new QFile(rutaImagen);
    if (!file->open(QIODevice::ReadOnly))
    {
        emit errorDeRed("Error abriendo imagen.");
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

void DataManager::guardarTicketCompletoServidor(
    const QJsonObject& jsonCompleto
    )
{
    cambiarEstadoRed(SINCRONIZANDO);

    QUrl url("http://161.97.92.143/api/v1/tickets/guardar");
    QNetworkRequest request(url);
    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );

    networkManager->post(request, QJsonDocument(jsonCompleto).toJson());
}

// ─────────────────────────────────────────
// REGISTRO USUARIO
// ─────────────────────────────────────────

void DataManager::registrarUsuarioRed(
    const QString& username,
    const QString& email,
    const QString& password
    )
{
    QUrl url("http://161.97.92.143/api/v1/usuarios/registro");
    QNetworkRequest request(url);
    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );

    QJsonObject json;
    json["nombre"]     = username;
    json["email"]      = email;
    json["clave_hash"] = hashPassword(password);

    networkManager->post(request, QJsonDocument(json).toJson());
}

// ─────────────────────────────────────────
// GUARDAR SUSCRIPCION SERVIDOR
// ─────────────────────────────────────────

void DataManager::guardarSuscripcionRed(
    const Suscripcion& s
    )
{
    cambiarEstadoRed(SINCRONIZANDO);

    int usuarioId = getUsuarioActivoId();

    if (usuarioId < 0)
    {
        qDebug() << "Usuario invalido";
        return;
    }

    QUrl url(
        "http://161.97.92.143/api/v1/suscripciones/guardar"
        );

    QNetworkRequest request(url);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );

    QJsonObject json;

    json["id_usuario"]   = usuarioId;
    json["id_categoria"] = 1;
    json["nombre"]       = s.nombreServicio;
    json["monto"]        = s.monto;
    json["moneda"]       = "ARS";
    json["frecuencia"]   = "mensual";
    json["vencimiento"]  = s.fechaVencimiento.toString(Qt::ISODate);
    json["alerta"]       = s.diasAviso;
    json["actividad"]    = s.activa ? 1 : 0;
    json["notas"]        = "";

    qDebug() << "ENVIANDO SUSCRIPCION:";
    qDebug() << QJsonDocument(json).toJson(QJsonDocument::Indented);

    networkManager->post(
        request,
        QJsonDocument(json).toJson()
        );
}

// ─────────────────────────────────────────
// TICKETS
// ─────────────────────────────────────────

QVector<Ticket> DataManager::getTickets(
    const QString& categoriaFiltro,
    const QString& busqueda
    )
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
        LEFT JOIN categorias c
            ON g.id_categoria = c.id_categoria
        WHERE g.id_usuario = :uid
        ORDER BY g.fecha_gasto DESC
    )");

    query.bindValue(":uid", usuarioId);

    if (!query.exec())
    {
        qDebug() << "Error getTickets:" << query.lastError().text();
        return tickets;
    }

    while (query.next())
    {
        Ticket t;
        t.id          = query.value("id_gasto").toInt();
        t.nombreLocal = query.value("comercio").toString();
        t.monto       = query.value("monto").toDouble();
        t.fecha       = QDate::fromString(
            query.value("fecha_gasto").toString(), Qt::ISODate
            );
        t.categoria      = query.value("categoria").toString();
        t.procesadoPorIA = false;
        t.imagenPath     = "";

        bool coincideCategoria =
            categoriaFiltro.isEmpty()
            || categoriaFiltro == "Todas las categorias"
            || t.categoria.contains(categoriaFiltro, Qt::CaseInsensitive);

        bool coincideBusqueda =
            busqueda.isEmpty()
            || t.nombreLocal.contains(busqueda, Qt::CaseInsensitive);

        if (coincideCategoria && coincideBusqueda)
            tickets.append(t);
    }

    return tickets;
}

bool DataManager::addTicket(const Ticket& t)
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0)
    {
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

    if (!query.exec())
    {
        qDebug() << "Error insertando ticket:"
                 << query.lastError().text();
        return false;
    }

    // --- ARMADO DEL JSON PARA EL VPS ---
    QJsonObject jsonRaiz;
    jsonRaiz.insert("id_usuario", usuarioId);

    // 1. Creamos el sub-objeto "gasto"
    QJsonObject gastoObj;
    gastoObj.insert("comercio", t.nombreLocal);
    gastoObj.insert("monto", t.monto);
    gastoObj.insert("fecha_gasto", t.fecha.toString(Qt::ISODate));
    gastoObj.insert("categoria_sugerida", t.categoria);
    gastoObj.insert("notas", "Generado localmente");

    // Lo metemos en la raíz
    jsonRaiz.insert("gasto", gastoObj);

    // 2. Creamos el sub-objeto "comprobante"
    QJsonObject compObj;
    if (t.imagenPath.isEmpty()) {
        compObj.insert("ruta_archivo", "carga_manual");
    } else {
        compObj.insert("ruta_archivo", t.imagenPath);
    }
    compObj.insert("estado", "procesado");

    // Lo metemos en la raíz
    jsonRaiz.insert("comprobante", compObj);

    // 3. NUEVO: ARMAMOS EL ARREGLO DE ITEMS PARA EL VPS
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

    // Enviamos al servidor
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

    if (!query.exec())
    {
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
            s.id_suscripcion_local AS id_suscripcion,
            s.nombre,
            s.monto,
            s.vencimiento,
            s.alerta,
            s.actividad,
            s.id_categoria_remota,
            c.nombre AS categoria
        FROM suscripciones s
        LEFT JOIN categorias c
            ON s.id_categoria_remota = c.id_categoria
        WHERE s.id_usuario_remoto = :uid
        ORDER BY s.vencimiento ASC
    )");

    query.bindValue(":uid", usuarioId);

    if (!query.exec())
    {
        qDebug() << "Error getSuscripciones:"
                 << query.lastError().text()
                 << query.lastQuery();
        return subs;
    }

    while (query.next())
    {
        Suscripcion s;
        s.id = query.value("id_suscripcion").toInt();
        s.nombreServicio = query.value("nombre").toString();
        s.monto = query.value("monto").toDouble();
        s.fechaVencimiento = QDate::fromString(query.value("vencimiento").toString(), Qt::ISODate);
        s.diasAviso = query.value("alerta").toInt();
        s.activa = query.value("actividad").toBool();
        s.iconoNombre = query.value("nombre").toString().toLower();
        s.categoria = query.value("categoria").toString();
        subs.append(s);
    }

    return subs;
}

bool DataManager::addSuscripcion(const Suscripcion& s)
{
    int usuarioId = getUsuarioActivoId();
    if (usuarioId < 0)
    {
        qDebug() << "No hay usuario activo.";
        return false;
    }

    // ELIMINAMOS EL INSERT EN SQLITE LOCAL.
    // Solo enviamos la suscripción al servidor. El VPS se encargará
    // de guardarla, y el próximo `/sync` (que disparamos en onRespuestaRecibida)
    // la bajará a nuestra base local, evitando duplicados.

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

    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        return false;
    }

    emit suscripcionesChanged();
    return true;
}

bool DataManager::removeSuscripcion(int id)
{
    int usuarioId = getUsuarioActivoId();

    QSqlQuery query(m_db.getDB());

    // cambie el DELETE por UPDATE para que en vez de borrar las
    //suscripciones de la base de datos las ponga en inactiva si la borra
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

    if (!query.exec())
    {
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
        WHERE s.id_usuario_remoto = :uid
        AND s.actividad = 1
        ORDER BY s.vencimiento ASC
    )");

    query.bindValue(":uid", usuarioId);

    if (query.exec() && query.next())
        return query.value(0).toInt();

    return 0;
}

// ─────────────────────────────────────────
// USUARIOS
// ─────────────────────────────────────────

bool DataManager::addUser(
    const QString& username,
    const QString& password
    )
{
    // guarda el hash en usuario_sesion para login offline futuro
    QSqlQuery query(m_db.getDB());

    query.prepare(R"(
        UPDATE usuario_sesion
        SET password_hash = :password,
            nombre        = :username
        WHERE sesion_activa = 1
    )");

    query.bindValue(":username", username);
    query.bindValue(":password", hashPassword(password));

    if (!query.exec())
    {
        qDebug() << "Error guardando password:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DataManager::login(
    const QString& username,
    const QString& password
    )
{
    // login local: fallback offline unicamente.
    // el login real se hace via loginRed() contra el vps.

    QSqlQuery query(m_db.getDB());

    query.prepare(R"(
        SELECT nombre, password_hash
        FROM usuario_sesion
        WHERE sesion_activa = 1
        LIMIT 1
    )");

    if (!query.exec() || !query.next())
    {
        qDebug() << "No hay sesion activa en sqlite (modo offline).";
        return false;
    }

    QString hashGuardado = query.value("password_hash").toString();

    // si no hay hash guardado todavia, aceptamos para el primer login offline
    if (hashGuardado.isEmpty())
        return true;

    return hashGuardado == hashPassword(password);
}

bool DataManager::userExists(
    const QString& username
    )
{
    QSqlQuery query(m_db.getDB());

    query.prepare(R"(
        SELECT nombre
        FROM usuario_sesion
        WHERE sesion_activa = 1
    )");

    if (!query.exec() || !query.next())
        return false;

    return query.value("nombre")
               .toString()
               .compare(username, Qt::CaseInsensitive) == 0;
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

QVector<QPair<QString,double>>
DataManager::getGastosPorCategoria(int year, int month)
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
    if (!query.exec(sql))
    {
        qDebug() << "Error getGastosPorCategoria:" << query.lastError().text();
        return resultado;
    }

    while (query.next())
        resultado.append(qMakePair(
            query.value("categoria").toString(),
            query.value("total").toDouble()
            ));

    return resultado;
}

QVector<QPair<QString,double>>
DataManager::getGastosPorSemana(int year, int month)
{
    QVector<QPair<QString,double>> resultado;

    int usuarioId = getUsuarioActivoId();
    QString monthStr = QString("%1").arg(month, 2, 10, QChar('0'));

    QString sql = QString(R"(
        SELECT
            ((CAST(strftime('%%d', fecha_gasto) AS INTEGER) - 1) / 7) + 1
                AS semana,
            SUM(monto) AS total
        FROM gastos
        WHERE id_usuario = %1
        AND strftime('%%Y', fecha_gasto) = '%2'
        AND strftime('%%m', fecha_gasto) = '%3'
        GROUP BY semana
        ORDER BY semana ASC
    )").arg(usuarioId).arg(year).arg(monthStr);

    QSqlQuery query(m_db.getDB());
    if (!query.exec(sql))
    {
        qDebug() << "Error getGastosPorSemana:" << query.lastError().text();
        return resultado;
    }

    while (query.next())
        resultado.append(qMakePair(
            QString("Semana %1").arg(query.value("semana").toInt()),
            query.value("total").toDouble()
            ));

    return resultado;
}

QSqlDatabase DataManager::getDB()
{
    return m_db.getDB();
}
// bool DataManager::updateSuscripcion(const Suscripcion &s)
// {
//     for (int i = 0; i < m_suscripciones.size(); i++) {

//         if (m_suscripciones[i].id == s.id) {

//             m_suscripciones[i] = s;

//             emit suscripcionesChanged();

//             return true;
//         }
//     }

//     return false;
// }
bool DataManager::updateSuscripcion(const Suscripcion &s)
{
    QSqlQuery query;

    query.prepare(R"(
        UPDATE suscripciones
        SET nombre_servicio = ?,
            monto = ?,
            fecha_vencimiento = ?,
            dias_aviso = ?,
            categoria = ?,
            icono_nombre = ?,
            sincronizado = 0,
            accion_pendiente = 'editar'
        WHERE id = ?
    )");

    query.addBindValue(s.nombreServicio);
    query.addBindValue(s.monto);
    query.addBindValue(s.fechaVencimiento.toString(Qt::ISODate));
    query.addBindValue(s.diasAviso);
    query.addBindValue(s.categoria);
    query.addBindValue(s.iconoNombre);
    query.addBindValue(s.id);

    bool ok = query.exec();

    if (!ok) {
        qDebug() << "Error updateSuscripcion:"
                 << query.lastError().text();
        return false;
    }

    emit suscripcionesChanged();

    return true;
}
QVector<Notificacion>
DataManager::getNotificaciones() const
{
    return m_notificaciones;
}

void DataManager::agregarNotificacion(
    const Notificacion &n
    )
{
    m_notificaciones.push_back(n);

    emit notificacionesChanged();
}

QString DataManager::getUltimoEmail() const
{
    QSettings settings("AlcancIA", "Login");
    return settings.value("ultimoEmail").toString();
}
//añado para que recuerde usuario al hacer click en la casilla de recordarme
void DataManager::setUltimoEmail(const QString &email)
{
    QSettings settings("AlcancIA", "Login");
    settings.setValue("ultimoEmail", email);
}