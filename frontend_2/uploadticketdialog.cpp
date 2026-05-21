#include "uploadticketdialog.h"
#include "stylemanager.h"
#include "models.h"
#include "datamanager.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QJsonArray>
#include <QFileInfo>

UploadTicketDialog::UploadTicketDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Subir Ticket");
    setModal(true);
    setFixedSize(560, 680);
    setStyleSheet(R"(
        QDialog {
            background-color: #0F1117;
            border: 1px solid #2E3347;
            border-radius: 16px;
        }
    )");
    setupUI();

    // Conectores del canal de respuestas asíncronas de la IA
    connect(&DataManager::instance(), &DataManager::estadoRedCambiado, this, [this](DataManager::EstadoRed estado) {
        if (estado == DataManager::ENVIANDO_FOTO) {
            m_analyzeBtn->setText("Analizando...");
            m_analyzeBtn->setEnabled(false);
            m_statusLabel->setText("⏳ La IA está procesando el ticket...");
            m_statusLabel->setStyleSheet("color: #818CF8; font-size: 12px; background: transparent;");
            m_statusLabel->show();
        } else if (estado == DataManager::EXITO || estado == DataManager::ERROR_CONEXION) {
            m_analyzeBtn->setText("✨  Analizar con IA");
            m_analyzeBtn->setEnabled(true);
        }
    });

    connect(&DataManager::instance(), &DataManager::ticketProcesadoRed, this, [this](const QString &comercio, double monto, const QString &fecha, const QString &categoria, const QJsonObject &jsonCompleto) {
        m_iaJsonResult = jsonCompleto; // Almacenamos el JSON completo de OpenAI

        m_localEdit->setText(comercio);
        m_montoSpin->setValue(monto);

        QDate parsedDate = QDate::fromString(fecha, Qt::ISODate);
        if(parsedDate.isValid()) m_fechaEdit->setDate(parsedDate);

        int idx = m_catCombo->findText(categoria, Qt::MatchContains);
        if(idx != -1) m_catCombo->setCurrentIndex(idx);

        m_statusLabel->setText("✅ ¡Datos completados por la IA!");
        m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 12px; background: transparent;");
    });

    connect(&DataManager::instance(), &DataManager::errorDeRed, this, [this](const QString &mensaje) {
        m_statusLabel->setText("❌ Error: " + mensaje);
        m_statusLabel->setStyleSheet("color: #F87171; font-size: 12px; background: transparent;");
        m_statusLabel->show();
    });
}

void UploadTicketDialog::setupUI() {
    QVBoxLayout *mainL = new QVBoxLayout(this);
    mainL->setContentsMargins(28, 24, 28, 24);
    mainL->setSpacing(20);

    QWidget *headerW = new QWidget();
    headerW->setStyleSheet("background: transparent;");
    QHBoxLayout *headerL = new QHBoxLayout(headerW);
    headerL->setContentsMargins(0,0,0,0);

    QWidget *titleW = new QWidget();
    titleW->setStyleSheet("background: transparent;");
    QVBoxLayout *titleL = new QVBoxLayout(titleW);
    titleL->setContentsMargins(0,0,0,0);
    titleL->setSpacing(2);

    QLabel *title = new QLabel("Subir Ticket");
    title->setStyleSheet("color: #F1F5F9; font-size: 20px; font-weight: 700; background: transparent;");
    QLabel *sub = new QLabel("La IA va a leer y clasificar automáticamente");
    sub->setStyleSheet("color: #64748B; font-size: 12px; background: transparent;");
    titleL->addWidget(title);
    titleL->addWidget(sub);

    QLabel *badge = new QLabel("✨ IA");
    badge->setStyleSheet("color: #818CF8; background-color: rgba(129,140,248,0.12); border: 1px solid rgba(129,140,248,0.3); border-radius: 12px; padding: 4px 10px; font-size: 11px; font-weight: 700;");

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(badge);
    mainL->addWidget(headerW);

    m_dropZone = new QFrame();
    m_dropZone->setFixedHeight(170);
    m_dropZone->setCursor(Qt::PointingHandCursor);
    m_dropZone->setStyleSheet("QFrame { background-color: rgba(74,222,128,0.03); border: 2px dashed #2E3347; border-radius: 14px; } QFrame:hover { border: 2px dashed #4ADE80; background-color: rgba(74,222,128,0.06); }");

    QVBoxLayout *dropL = new QVBoxLayout(m_dropZone);
    dropL->setAlignment(Qt::AlignCenter);
    dropL->setSpacing(8);

    m_dropIcon = new QLabel("📄");
    m_dropIcon->setStyleSheet("font-size: 40px; background: transparent;");
    m_dropIcon->setAlignment(Qt::AlignCenter);

    m_dropLabel = new QLabel("Arrastrá una imagen o PDF acá");
    m_dropLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 600; background: transparent;");
    m_dropLabel->setAlignment(Qt::AlignCenter);

    m_dropSub = new QLabel("JPG, PNG, PDF — máx 10 MB");
    m_dropSub->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");
    m_dropSub->setAlignment(Qt::AlignCenter);

    m_selectBtn = new QPushButton("Seleccionar archivo");
    m_selectBtn->setFixedSize(160, 34);
    m_selectBtn->setCursor(Qt::PointingHandCursor);
    m_selectBtn->setStyleSheet("QPushButton { background-color: #21253A; color: #94A3B8; border: 1px solid #2E3347; border-radius: 8px; font-size: 12px; font-weight: 600; } QPushButton:hover { border: 1px solid #4ADE80; color: #4ADE80; }");
    connect(m_selectBtn, &QPushButton::clicked, this, &UploadTicketDialog::onSelectImage);

    dropL->addWidget(m_dropIcon);
    dropL->addWidget(m_dropLabel);
    dropL->addWidget(m_dropSub);
    dropL->addSpacing(8);
    dropL->addWidget(m_selectBtn, 0, Qt::AlignCenter);

    mainL->addWidget(m_dropZone);

    m_analyzeBtn = new QPushButton("✨  Analizar con IA");
    m_analyzeBtn->setFixedHeight(42);
    m_analyzeBtn->setEnabled(false);
    m_analyzeBtn->setCursor(Qt::PointingHandCursor);
    m_analyzeBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #818CF8, stop:1 #6366F1); color: white; border: none; border-radius: 10px; font-size: 13px; font-weight: 700; } QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366F1, stop:1 #4F46E5); } QPushButton:disabled { background-color: #21253A; color: #475569; }");
    connect(m_analyzeBtn, &QPushButton::clicked, this, &UploadTicketDialog::onAnalyzeIA);
    mainL->addWidget(m_analyzeBtn);

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 12px; background: transparent;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    mainL->addWidget(m_statusLabel);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #2E3347; border: none;");
    mainL->addWidget(sep);

    QLabel *formTitle = new QLabel("Datos del ticket");
    formTitle->setStyleSheet("color: #94A3B8; font-size: 12px; font-weight: 600; letter-spacing: 0.5px; background: transparent;");
    mainL->addWidget(formTitle);

    QWidget *formW = new QWidget();
    formW->setStyleSheet("background: transparent;");
    QGridLayout *formL = new QGridLayout(formW);
    formL->setContentsMargins(0,0,0,0);
    formL->setSpacing(12);

    QString inputStyle = R"(
        QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit {
            background-color: #21253A; color: #F1F5F9;
            border: 1px solid #2E3347; border-radius: 8px;
            padding: 8px 12px; font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QDateEdit:focus { border: 1px solid #4ADE80; }
    )";

    auto makeLabel = [](const QString &txt) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet("color: #64748B; font-size: 12px; background: transparent;");
        return l;
    };

    m_localEdit = new QLineEdit();
    m_localEdit->setPlaceholderText("ej: Disco Supermarket");
    m_localEdit->setStyleSheet(inputStyle);

    m_montoSpin = new QDoubleSpinBox();
    m_montoSpin->setPrefix("$ ");
    m_montoSpin->setMaximum(9999999.99);
    m_montoSpin->setDecimals(2);
    m_montoSpin->setStyleSheet(inputStyle);

    m_fechaEdit = new QDateEdit(QDate::currentDate());
    m_fechaEdit->setCalendarPopup(true);
    m_fechaEdit->setStyleSheet(inputStyle);

    m_catCombo = new QComboBox();
    for (auto &c : CATEGORIAS) m_catCombo->addItem(c);
    m_catCombo->setStyleSheet(inputStyle);

    formL->addWidget(makeLabel("Local / Comercio"),   0, 0);
    formL->addWidget(m_localEdit,                     1, 0);
    formL->addWidget(makeLabel("Monto Total"),         0, 1);
    formL->addWidget(m_montoSpin,                     1, 1);
    formL->addWidget(makeLabel("Fecha"),               2, 0);
    formL->addWidget(m_fechaEdit,                     3, 0);
    formL->addWidget(makeLabel("Categoría"),           2, 1);
    formL->addWidget(m_catCombo,                      3, 1);

    mainL->addWidget(formW);

    mainL->addStretch();
    QWidget *btnW = new QWidget();
    btnW->setStyleSheet("background: transparent;");
    QHBoxLayout *btnL = new QHBoxLayout(btnW);
    btnL->setContentsMargins(0,0,0,0);
    btnL->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setFixedHeight(40);
    m_cancelBtn->setStyleSheet("QPushButton { background: transparent; color: #64748B; border: 1px solid #2E3347; border-radius: 8px; font-size: 13px; } QPushButton:hover { color: #94A3B8; border-color: #475569; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_saveBtn = new QPushButton("💾  Guardar Ticket");
    m_saveBtn->setFixedHeight(40);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #4ADE80; color: #0F1117; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; } QPushButton:hover { background-color: #22C55E; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &UploadTicketDialog::accept);

    btnL->addWidget(m_cancelBtn, 1);
    btnL->addWidget(m_saveBtn, 2);
    mainL->addWidget(btnW);
}

void UploadTicketDialog::onSelectImage() {
    QString path = QFileDialog::getOpenFileName(this, "Seleccionar imagen o PDF", QDir::homePath(), "Imágenes y PDFs (*.jpg *.jpeg *.png *.pdf)");
    if (!path.isEmpty()) {
        m_imagenPath = path;
        updateDropZone(path);
        m_analyzeBtn->setEnabled(true);
    }
}

void UploadTicketDialog::updateDropZone(const QString &path) {
    QFileInfo fi(path);
    m_dropIcon->setText("✅");
    m_dropLabel->setText(fi.fileName());
    m_dropLabel->setStyleSheet("color: #4ADE80; font-size: 13px; font-weight: 600; background: transparent;");
    m_dropSub->setText(QString("%1 KB").arg(fi.size() / 1024));
    m_dropZone->setStyleSheet("QFrame { background-color: rgba(74,222,128,0.06); border: 2px dashed #4ADE80; border-radius: 14px; }");
    m_selectBtn->setText("Cambiar archivo");
}

void UploadTicketDialog::onAnalyzeIA() {
    if (!m_imagenPath.isEmpty()) {
        DataManager::instance().analizarTicketRed(m_imagenPath);
    }
}

void UploadTicketDialog::accept() {
    QJsonObject payload;

    // Si m_iaJsonResult está vacío, construimos el esquema base de forma manual
    if (m_iaJsonResult.isEmpty()) {
        QJsonObject comprobante;
        comprobante["ruta_archivo"] = m_imagenPath.isEmpty() ? "tickets/manual.jpg" : m_imagenPath;
        comprobante["estado"] = "procesado";

        QJsonObject gasto;
        gasto["comercio"] = m_localEdit->text();
        gasto["monto"] = m_montoSpin->value();
        gasto["fecha_gasto"] = m_fechaEdit->date().toString(Qt::ISODate);
        gasto["categoria_sugerida"] = m_catCombo->currentText();
        gasto["notas"] = "Carga manual desde la app de escritorio";

        payload["comprobante"] = comprobante;
        payload["gasto"] = gasto;
        payload["items_gasto"] = QJsonArray();
        payload["suscripcion"] = QJsonValue::Null;
        payload["notificacion"] = QJsonValue::Null;
    } else {
        // Si usamos la IA, respetamos la respuesta pero actualizamos por si editaste los campos en la ventana
        payload = m_iaJsonResult;
        QJsonObject gasto = payload["gasto"].toObject();
        gasto["comercio"] = m_localEdit->text();
        gasto["monto"] = m_montoSpin->value();
        gasto["fecha_gasto"] = m_fechaEdit->date().toString(Qt::ISODate);
        gasto["categoria_sugerida"] = m_catCombo->currentText();
        payload["gasto"] = gasto;
    }

    payload["id_usuario"] = 1; // El ID demo que maneja tu Python

    // 1. Enviamos el paquete completo al VPS (Tu API Python)
    DataManager::instance().guardarTicketCompletoServidor(payload);

    // 2. Guardamos también en el archivo local JSON de la Mac para que aparezca al instante
    Ticket t;
    t.nombreLocal = m_localEdit->text();
    t.monto = m_montoSpin->value();
    t.fecha = m_fechaEdit->date();
    t.categoria = m_catCombo->currentText();
    t.imagenPath = m_imagenPath;
    t.procesadoPorIA = !m_iaJsonResult.isEmpty();
    DataManager::instance().addTicket(t);

    QDialog::accept();
}

QString     UploadTicketDialog::imagenPath()  const { return m_imagenPath; }
QString     UploadTicketDialog::nombreLocal() const { return m_localEdit->text(); }
double      UploadTicketDialog::monto()       const { return m_montoSpin->value(); }
QDate       UploadTicketDialog::fecha()       const { return m_fechaEdit->date(); }
QString     UploadTicketDialog::categoria()   const { return m_catCombo->currentText(); }