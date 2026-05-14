#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QHBoxLayout>
#include "sidebar.h"
#include "dashboardpage.h"
#include "ticketspage.h"
#include "subscriptionspage.h"
#include "reportspage.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

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