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

class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() = default;

private slots:
    void onLogout();
    void onRefreshTimer();
    void onNotificacionesClicked();
    void updateNotifBadge();

private:
    void setupUI();
    void applyStyles();
    void refreshAllPages();
    QWidget* buildNotifPanel();

    Sidebar            *m_sidebar       = nullptr;
    QStackedWidget     *m_stack         = nullptr;
    DashboardPage      *m_dashboard     = nullptr;
    TicketsPage        *m_tickets       = nullptr;
    SubscriptionsPage  *m_subs          = nullptr;
    ReportsPage        *m_reports       = nullptr;
    QTimer             *m_refreshTimer  = nullptr;

    // Notificaciones
    QPushButton        *m_notifBtn      = nullptr;
    QLabel             *m_notifBadge    = nullptr;
    QWidget            *m_notifPanel    = nullptr;
    QVBoxLayout        *m_notifListLayout = nullptr;
    bool                m_notifVisible  = false;
};