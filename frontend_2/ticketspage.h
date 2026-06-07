#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>

#include "models.h"

/*
 * TicketCard
 * Responsabilidad de clase:
 * Componente visual para un gasto individual.
 * Expone una senal de borrado para que la pagina controle la accion.
 *
 * SIGNAL:
 * deleteRequested() comunica la intencion del usuario sin borrar directamente.
 */
class TicketCard : public QFrame {
    Q_OBJECT

public:
    explicit TicketCard(
        const Ticket &ticket,
        QWidget *parent = nullptr
        );

signals:
    void deleteRequested(int idRemoto);
};

/*
 * TicketsPage
 * Responsabilidad de clase:
 * Pantalla de administracion de gastos/tickets.
 * Permite listar, buscar, filtrar, subir comprobantes y eliminar registros.
 *
 * SLOT:
 * onUploadClicked() y applyFilter() reaccionan a eventos de botones/campos UI.
 */
class TicketsPage : public QWidget {
    Q_OBJECT

public:
    explicit TicketsPage(QWidget *parent = nullptr);

    // Recarga la lista visible desde DataManager aplicando filtros actuales.
    void refreshData();

public slots:
    void onUploadClicked();

private slots:
    void applyFilter();

private:
    void setupUI();
    void addTicketCard(const Ticket &t);

private:
    // Atributos importantes: controles usados para reconstruir la lista
    // y conservar el filtro/busqueda entre refrescos.
    QVBoxLayout *m_ticketListLayout = nullptr;
    QWidget *m_ticketListWidget = nullptr;

    QLabel *m_countLabel = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_filterCombo = nullptr;
};
