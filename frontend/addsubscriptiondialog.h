#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

class AddSubscriptionDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddSubscriptionDialog(QWidget *parent = nullptr);

    QString nombreServicio() const;
    double  monto() const;
    QDate   fechaVencimiento() const;
    int     diasAviso() const;
    QString categoria() const;
    QString iconoNombre() const;

private:
    void setupUI();
    void updateIconPreview(const QString &nombre);

    QLineEdit      *m_nombreEdit    = nullptr;
    QDoubleSpinBox *m_montoSpin     = nullptr;
    QDateEdit      *m_fechaEdit     = nullptr;
    QSpinBox       *m_diasSpin      = nullptr;
    QComboBox      *m_catCombo      = nullptr;
    QPushButton    *m_saveBtn       = nullptr;
    QPushButton    *m_cancelBtn     = nullptr;
};
