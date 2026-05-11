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

class UploadTicketDialog : public QDialog {
    Q_OBJECT
public:
    explicit UploadTicketDialog(QWidget *parent = nullptr);

    QString     imagenPath() const;
    QString     nombreLocal() const;
    double      monto() const;
    QDate       fecha() const;
    QString     categoria() const;

private slots:
    void onSelectImage();
    void onAnalyzeIA();

private:
    void setupUI();
    void updateDropZone(const QString &path);

    QLabel         *m_dropIcon      = nullptr;
    QLabel         *m_dropLabel     = nullptr;
    QLabel         *m_dropSub       = nullptr;
    QFrame         *m_dropZone      = nullptr;
    QLabel         *m_previewLabel  = nullptr;
    QPushButton    *m_selectBtn     = nullptr;
    QPushButton    *m_analyzeBtn    = nullptr;
    QLineEdit      *m_localEdit     = nullptr;
    QDoubleSpinBox *m_montoSpin     = nullptr;
    QDateEdit      *m_fechaEdit     = nullptr;
    QComboBox      *m_catCombo      = nullptr;
    QPushButton    *m_saveBtn       = nullptr;
    QPushButton    *m_cancelBtn     = nullptr;
    QLabel         *m_statusLabel   = nullptr;

    QString m_imagenPath;
};
