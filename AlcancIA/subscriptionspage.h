#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

#include "models.h"

/*
 * SubCard
 * Responsabilidad de clase:
 * Tarjeta visual de una suscripcion.
 * Usa senales para pedir activar/desactivar, eliminar o editar sin conocer
 * directamente la logica de datos.
 *
 * SIGNAL:
 * Las senales separan la tarjeta visual de la logica de persistencia.
 */
class SubCard : public QFrame
{
    Q_OBJECT

public:
    explicit SubCard(
        const Suscripcion &sub,
        QWidget *parent = nullptr
        );

signals:
    void toggleRequested(
        int idRemoto,
        bool activa
        );

    void deleteRequested(
        int idRemoto
        );

    void editRequested(
        int idRemoto
        );
};

/*
 * SubscriptionsPage
 * Responsabilidad de clase:
 * Pantalla para gestionar suscripciones recurrentes.
 * Muestra totales, permite crear nuevas suscripciones y editar o desactivar
 * las existentes.
 *
 * Validaciones:
 * La creacion/edicion se apoya en AddSubscriptionDialog, donde se validan
 * campos antes de guardar.
 */
class SubscriptionsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SubscriptionsPage(
        QWidget *parent = nullptr
        );

    // Recarga las suscripciones activas desde DataManager.
    void refreshData();

public slots:
    void onAddClicked();

private slots:
    void onEditClicked(int id);

private:
    void setupUI();

    void addSubCard(
        const Suscripcion &s
        );

private:
    // Atributos importantes: lista y totales se actualizan cada vez
    // que cambian las suscripciones.
    QVBoxLayout *m_subListLayout = nullptr;

    QWidget *m_subListWidget = nullptr;

    QLabel *m_totalLabel = nullptr;

    QLabel *m_countLabel = nullptr;
};
