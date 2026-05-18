#include "mainwindow.h"
#include "stylemanager.h"
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("AlcancIA — Asistente Financiero");
    setMinimumSize(1100, 680);
    resize(1280, 780);
    applyStyles();
    setupUI();
}

void MainWindow::applyStyles() {
    setStyleSheet(StyleManager::appStyleSheet() + R"(
        QMainWindow { background-color: #0F1117; }
    )");
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #0F1117;");
    setCentralWidget(central);

    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_sidebar = new Sidebar(central);

    QWidget *contentArea = new QWidget();
    contentArea->setStyleSheet("background-color: #0F1117;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new QStackedWidget();
    m_dashboard = new DashboardPage();
    m_tickets = new TicketsPage();
    m_subs = new SubscriptionsPage();
    m_reports = new ReportsPage();

    m_stack->addWidget(m_dashboard);
    m_stack->addWidget(m_tickets);
    m_stack->addWidget(m_subs);
    m_stack->addWidget(m_reports);

    contentLayout->addWidget(m_stack);

    connect(m_sidebar, &Sidebar::pageChanged, this, [this](int idx) {
        m_stack->setCurrentIndex(idx);
        // Refresh data when page is shown
        if (idx == 0) m_dashboard->refreshData();
        else if (idx == 1) m_tickets->refreshData();
        else if (idx == 2) m_subs->refreshData();
        else if (idx == 3) m_reports->refreshData();
    });

    connect(m_dashboard, &DashboardPage::navigateToTickets, this, [this]() {
        m_sidebar->setActivePage(1);
        m_stack->setCurrentIndex(1);
        m_tickets->onUploadClicked();
    });
    connect(m_dashboard, &DashboardPage::navigateToSubs, this, [this]() {
        m_sidebar->setActivePage(2);
        m_stack->setCurrentIndex(2);
        m_subs->onAddClicked();
    });

    rootLayout->addWidget(m_sidebar);
    rootLayout->addWidget(contentArea, 1);
}