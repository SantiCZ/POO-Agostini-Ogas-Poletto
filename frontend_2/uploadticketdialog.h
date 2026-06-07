#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QJsonObject>

/*
 * UploadTicketDialog
 * Responsabilidad de clase:
 * Dialogo para cargar un comprobante manualmente o analizar una imagen con IA.
 * Al confirmar, envia los datos a DataManager para persistencia local/remota.
 *
 * Herencia y polimorfismo:
 * Hereda de QDialog y sobrescribe accept() para validar y transformar datos
 * antes de cerrar el dialogo.
 */
class UploadTicketDialog : public QDialog {
    Q_OBJECT

public:
    explicit UploadTicketDialog(QWidget *parent = nullptr);

    // Validaciones: verifica comercio, monto y sesion activa.
    void accept() override;

    QString imagenPath() const;
    QString nombreLocal() const;
    double monto() const;
    QDate fecha() const;
    QString categoria() const;

private slots:
    // Exportaciones/archivos: no exporta datos; abre un archivo de
    // imagen local con QFileDialog para asociarlo al ticket.
    void onSelectImage();

    // Solicita al servidor que interprete la imagen del ticket.
    void onAnalyzeIA();

    // Completa el formulario con la respuesta del VPS.
    void onTicketProcesadoIA(const QString &comercio,
                             double monto,
                             const QString &fecha,
                             const QString &categoria,
                             const QJsonObject &jsonCompleto);

    void onErrorRed(const QString &mensaje);

private:
    void setupUI();

    // Actualiza el texto visual del area de carga despues de elegir archivo.
    void updateDropZone(const QString &path);

    QLabel         *m_dropIcon      = nullptr;
    QLabel         *m_dropLabel     = nullptr;
    QLabel         *m_dropSub       = nullptr;

    QFrame         *m_dropZone      = nullptr;

    QPushButton    *m_selectBtn     = nullptr;
    QPushButton    *m_analyzeBtn    = nullptr;

    QLineEdit      *m_localEdit     = nullptr;

    QDoubleSpinBox *m_montoSpin     = nullptr;

    QDateEdit      *m_fechaEdit     = nullptr;

    QComboBox      *m_catCombo      = nullptr;

    QPushButton    *m_saveBtn       = nullptr;
    QPushButton    *m_cancelBtn     = nullptr;

    QLabel         *m_statusLabel   = nullptr;

    QString     m_imagenPath;

    // Respuesta completa del VPS para poder guardar comprobante, gasto e items.
    QJsonObject m_iaJsonResult;
};
