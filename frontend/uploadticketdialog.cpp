#include "uploadticketdialog.h"
#include "stylemanager.h"
#include "models.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QTimer>

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
}

void UploadTicketDialog::setupUI() {
    QVBoxLayout *mainL = new QVBoxLayout(this);
    mainL->setContentsMargins(28, 24, 28, 24);
    mainL->setSpacing(20);

    // ── Encabezado ────────────────────────────────────────────────
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
    badge->setStyleSheet(R"(
        color: #818CF8; background-color: rgba(129,140,248,0.12);
        border: 1px solid rgba(129,140,248,0.3);
        border-radius: 12px; padding: 4px 10px;
        font-size: 11px; font-weight: 700;
    )");

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(badge);
    mainL->addWidget(headerW);

    // ── Zona de drop ──────────────────────────────────────────────
    m_dropZone = new QFrame();
    m_dropZone->setFixedHeight(170);
    m_dropZone->setCursor(Qt::PointingHandCursor);
    m_dropZone->setStyleSheet(R"(
        QFrame {
            background-color: rgba(74,222,128,0.03);
            border: 2px dashed #2E3347;
            border-radius: 14px;
        }
        QFrame:hover {
            border: 2px dashed #4ADE80;
            background-color: rgba(74,222,128,0.06);
        }
    )");

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
    m_selectBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #21253A;
            color: #94A3B8;
            border: 1px solid #2E3347;
            border-radius: 8px;
            font-size: 12px;
            font-weight: 600;
        }
        QPushButton:hover {
            border: 1px solid #4ADE80;
            color: #4ADE80;
        }
    )");
    connect(m_selectBtn, &QPushButton::clicked, this, &UploadTicketDialog::onSelectImage);

    dropL->addWidget(m_dropIcon);
    dropL->addWidget(m_dropLabel);
    dropL->addWidget(m_dropSub);
    dropL->addSpacing(8);
    dropL->addWidget(m_selectBtn, 0, Qt::AlignCenter);

    mainL->addWidget(m_dropZone);

    // ── Botón analizar ────────────────────────────────────────────
    m_analyzeBtn = new QPushButton("✨  Analizar con IA");
    m_analyzeBtn->setFixedHeight(42);
    m_analyzeBtn->setEnabled(false);
    m_analyzeBtn->setCursor(Qt::PointingHandCursor);
    m_analyzeBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #818CF8, stop:1 #6366F1);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6366F1, stop:1 #4F46E5);
        }
        QPushButton:disabled {
            background-color: #21253A;
            color: #475569;
        }
    )");
    connect(m_analyzeBtn, &QPushButton::clicked, this, &UploadTicketDialog::onAnalyzeIA);
    mainL->addWidget(m_analyzeBtn);

    // ── Status ────────────────────────────────────────────────────
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #4ADE80; font-size: 12px; background: transparent;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    mainL->addWidget(m_statusLabel);

    // ── Separador ─────────────────────────────────────────────────
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #2E3347; border: none;");
    mainL->addWidget(sep);

    // ── Formulario manual ─────────────────────────────────────────
    QLabel *formTitle = new QLabel("Datos del ticket");
    formTitle->setStyleSheet("color: #94A3B8; font-size: 12px; font-weight: 600; "
                              "letter-spacing: 0.5px; background: transparent;");
    mainL->addWidget(formTitle);

    QWidget *formW = new QWidget();
    formW->setStyleSheet("background: transparent;");
    QGridLayout *formL = new QGridLayout(formW);
    formL->setContentsMargins(0,0,0,0);
    formL->setSpacing(12);

    QString inputStyle = R"(
        QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {
            border: 1px solid #4ADE80;
        }
        QComboBox QAbstractItemView {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            selection-background-color: #2E3347;
        }
        QComboBox::drop-down { border: none; width: 24px; }
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background: #2E3347; border: none; border-radius: 3px;
        }
        QDateEdit::drop-down { border: none; width: 24px; }
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

    // ── Botones finales ───────────────────────────────────────────
    mainL->addStretch();
    QWidget *btnW = new QWidget();
    btnW->setStyleSheet("background: transparent;");
    QHBoxLayout *btnL = new QHBoxLayout(btnW);
    btnL->setContentsMargins(0,0,0,0);
    btnL->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setFixedHeight(40);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent; color: #64748B;
            border: 1px solid #2E3347; border-radius: 8px;
            font-size: 13px;
        }
        QPushButton:hover { color: #94A3B8; border-color: #475569; }
    )");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_saveBtn = new QPushButton("💾  Guardar Ticket");
    m_saveBtn->setFixedHeight(40);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4ADE80;
            color: #0F1117;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover { background-color: #22C55E; }
    )");
    connect(m_saveBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnL->addWidget(m_cancelBtn, 1);
    btnL->addWidget(m_saveBtn, 2);
    mainL->addWidget(btnW);
}

void UploadTicketDialog::onSelectImage() {
    QString path = QFileDialog::getOpenFileName(
        this, "Seleccionar imagen o PDF",
        QDir::homePath(),
        "Imágenes y PDFs (*.jpg *.jpeg *.png *.pdf)"
    );
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
    m_dropZone->setStyleSheet(R"(
        QFrame {
            background-color: rgba(74,222,128,0.06);
            border: 2px dashed #4ADE80;
            border-radius: 14px;
        }
    )");
    m_selectBtn->setText("Cambiar archivo");
}

void UploadTicketDialog::onAnalyzeIA() {
    // Simulación de análisis IA (sin implementar aún)
    m_analyzeBtn->setText("Analizando...");
    m_analyzeBtn->setEnabled(false);
    m_statusLabel->setText("⏳ La IA está procesando el ticket...");
    m_statusLabel->show();

    QTimer::singleShot(1500, this, [this]() {
        // Datos de ejemplo — en producción vendrán de la API
        m_localEdit->setText("Disco Supermarket");
        m_montoSpin->setValue(4850.00);
        m_fechaEdit->setDate(QDate::currentDate());
        m_catCombo->setCurrentText("Supermercado");

        m_statusLabel->setText("✅ ¡Datos completados por la IA!");
        m_analyzeBtn->setText("✨  Analizar con IA");
        m_analyzeBtn->setEnabled(true);
    });
}

QString     UploadTicketDialog::imagenPath()  const { return m_imagenPath; }
QString     UploadTicketDialog::nombreLocal() const { return m_localEdit->text(); }
double      UploadTicketDialog::monto()       const { return m_montoSpin->value(); }
QDate       UploadTicketDialog::fecha()       const { return m_fechaEdit->date(); }
QString     UploadTicketDialog::categoria()   const { return m_catCombo->currentText(); }
