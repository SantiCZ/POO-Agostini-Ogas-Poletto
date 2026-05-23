#include "datamanager.h"
#include "admindb.h" // NUEVO: Importamos para hablar con SQLite
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMap>
#include <QFileInfo>

// ─── INCLUDES DE RED (Corregidos para Mac) ──────────────────────
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

DataManager::DataManager() : QObject(nullptr), estadoActual(ESPERANDO) {
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &DataManager::onRespuestaRecibida);
}

DataManager& DataManager::instance() {
    static DataManager inst;
    return inst;
}

// NUEVO: Función para sincronizar con el servidor
void DataManager::sincronizarDesdeServidor(int id_usuario) {
    cambiarEstadoRed(SINCRONIZANDO);
    // Llamamos al nuevo endpoint de sincronización que definió Avril
    QUrl url(QString("http://161.97.92.143/api/v1/usuarios/%1/sync").arg(id_usuario));
    QNetworkRequest request(url);
    networkManager->get(request);
}

// ─── LÓGICA DE PERSISTENCIA (El corazón del cambio) ──────────────
void DataManager::onRespuestaRecibida(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorDeRed("Error de red: " + reply->errorString());
        cambiarEstadoRed(ERROR_CONEXION);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QString path = reply->url().path();

    // NUEVO: Si recibimos la respuesta de /sync, guardamos en SQLite
    if (path.contains("/sync")) {
        adminDB db;
        // Obtenemos la ruta del proyecto dinámicamente
        QString rutaDB = QFileInfo(__FILE__).absolutePath() + "/tasty_alcancia.db";

        if (db.conectar(rutaDB)) {
            // sincronizarDesdeJson ahora se encarga de: DELETE (limpiar) + INSERT (guardar)
            if (db.sincronizarDesdeJson(responseData)) {
                qDebug() << "Sincronización a SQLite exitosa.";
                emit sincronizacionCompletada(); // Avisamos a la UI para que refresque las tablas
            }
        }
        cambiarEstadoRed(EXITO);
    }
    // Mantenemos la lógica original para otras peticiones (tickets, registro, etc)
    else if (path.contains("/analizar")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        QJsonObject gastoObj = root["gasto"].toObject();
        emit ticketProcesadoRed(gastoObj["comercio"].toString(), gastoObj["monto"].toDouble(), gastoObj["fecha_gasto"].toString(), gastoObj["categoria_sugerida"].toString(), root);
        cambiarEstadoRed(EXITO);
    } else if (path.contains("/guardar")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        emit ticketGuardadoServidor(root["status"].toString() == "ok", root["message"].toString());
    } else if (path.contains("/registro")) {
        QJsonObject root = QJsonDocument::fromJson(responseData).object();
        emit usuarioRegistradoServidor(root["status"].toString() == "ok", root["message"].toString());
    }

    reply->deleteLater();
}

// ─── FUNCIONES DE RED (Corregidas sintaxis Qt6) ──────────────────
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

void DataManager::registrarUsuarioRed(const QString &username, const QString &email, const QString &password) {
    cambiarEstadoRed(ESPERANDO);
    QUrl url("http://161.97.92.143/api/v1/usuarios/registro");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject jsonUser;
    // CORRECCIÓN: Usamos .insert() para compatibilidad absoluta con Qt6
    jsonUser.insert("nombre", username);
    jsonUser.insert("email", email);
    jsonUser.insert("clave_hash", hashPassword(password));

    QJsonDocument doc(jsonUser);
    networkManager->post(request, doc.toJson());
}

QString DataManager::hashPassword(const QString& pwd) const {
    return QString(QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5).toHex());
}