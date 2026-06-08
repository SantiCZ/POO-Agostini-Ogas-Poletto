#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>

#include "sidebar.h"
#include "dashboardpage.h"
#include "ticketspage.h"
#include "subscriptionspage.h"
#include "reportspage.h"

/*
 * MainWidget
 * Responsabilidad de clase:
 * Ventana principal luego del login.
 * Coordina el sidebar, las paginas internas, el refresco periodico y el
 * cierre seguro con sincronizacion de datos pendientes.
 *
 * Herencia:
 * Hereda de QWidget porque funciona como ventana contenedora de otras paginas.
 *
 * Polimorfismo:
 * Sobrescribe closeEvent() para agregar sincronizacion antes del cierre.
 */
class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() = default;

protected:
    // Antes de cerrar intenta sincronizar cambios locales pendientes.
    void closeEvent(QCloseEvent *event) override;   // NUEVO

private slots:
    // SLOT: responden a botones, timer y cambios de notificaciones.
    // Acciones globales disparadas desde la interfaz.
    void onLogout();
    void onRefreshTimer();
    void onNotificacionesClicked();
    void updateNotifBadge();

private:
    // Construccion visual y refresco de las paginas del stacked widget.
    void setupUI();
    void applyStyles();
    void refreshAllPages();
    QWidget* buildNotifPanel();

    // Atributos importantes: componentes principales de navegacion
    // y contenido. Se guardan como miembros porque se actualizan durante toda
    // la ejecucion de la ventana.
    Sidebar            *m_sidebar       = nullptr;
    QStackedWidget     *m_stack         = nullptr;
    DashboardPage      *m_dashboard     = nullptr;
    TicketsPage        *m_tickets       = nullptr;
    SubscriptionsPage  *m_subs          = nullptr;
    ReportsPage        *m_reports       = nullptr;
    QTimer             *m_refreshTimer  = nullptr;

    // Atributos importantes: estado visual del panel de notificaciones.
    QPushButton        *m_notifBtn        = nullptr;
    QLabel             *m_notifBadge      = nullptr;
    QWidget            *m_notifPanel      = nullptr;
    QVBoxLayout        *m_notifListLayout = nullptr;
    bool                m_notifVisible    = false;
};
