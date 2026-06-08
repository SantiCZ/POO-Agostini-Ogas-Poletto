#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

/*
 * AddSubscriptionDialog
 * Responsabilidad de clase:
 * Dialogo modal para crear o validar datos de una suscripcion.
 * Expone getters para que SubscriptionsPage construya el modelo Suscripcion.
 *
 * Herencia y polimorfismo:
 * Hereda de QDialog y sobrescribe accept(), metodo virtual que se ejecuta al
 * confirmar el dialogo. Asi se validan los campos antes de cerrar.
 */
class AddSubscriptionDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddSubscriptionDialog(QWidget *parent = nullptr);

    // Validaciones: intercepta el guardado para revisar campos.
    void accept() override; // NUEVO: Interceptamos el guardado

    QString nombreServicio() const;
    double  monto() const;
    QDate   fechaVencimiento() const;
    int     diasAviso() const;
    QString iconoNombre() const;

private:
    void setupUI();

    // Ajusta la vista previa del icono segun el servicio elegido.
    void updateIconPreview(const QString &nombre);

    QLineEdit      *m_nombreEdit    = nullptr;
    QDoubleSpinBox *m_montoSpin     = nullptr;
    QDateEdit      *m_fechaEdit     = nullptr;
    QSpinBox       *m_diasSpin      = nullptr;
    QPushButton    *m_saveBtn       = nullptr;
    QPushButton    *m_cancelBtn     = nullptr;
};
