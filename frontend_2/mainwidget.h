#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
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

private:
    void setupUI();
    void applyStyles();

    Sidebar            *m_sidebar      = nullptr;
    QStackedWidget     *m_stack        = nullptr;
    DashboardPage      *m_dashboard    = nullptr;
    TicketsPage        *m_tickets      = nullptr;
    SubscriptionsPage  *m_subs         = nullptr;
    ReportsPage        *m_reports      = nullptr;
};
