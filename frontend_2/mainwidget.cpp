#include "mainwidget.h"
#include "stylemanager.h"
#include "logindialog.h"
#include "datamanager.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent) {

    setWindowTitle("AlcancIA — Asistente Financiero");
    setMinimumSize(1100, 680);
    resize(1280, 780);

    applyStyles();
    setupUI();

    // CORREGIDO: el timer solo refresca la UI local (SQLite),
    // no llama sincronizarDesdeServidor() para no duplicar datos.
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWidget::onRefreshTimer);
    m_refreshTimer->start(30000);
}

void MainWidget::applyStyles() {
    setStyleSheet(
        StyleManager::appStyleSheet() + R"(
        QWidget {
            background-color: #0F1117;
        }
    )");
}

void MainWidget::setupUI() {

    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_sidebar = new Sidebar(this);
    connect(m_sidebar, &Sidebar::logoutRequested, this, &MainWidget::onLogout);

    QWidget *contentArea = new QWidget();
    contentArea->setStyleSheet("background-color: #0F1117;");

    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new QStackedWidget();

    m_dashboard = new DashboardPage();
    m_tickets   = new TicketsPage();
    m_subs      = new SubscriptionsPage();
    m_reports   = new ReportsPage();

    m_stack->addWidget(m_dashboard); // 0
    m_stack->addWidget(m_tickets);   // 1
    m_stack->addWidget(m_subs);      // 2
    m_stack->addWidget(m_reports);   // 3

    contentLayout->addWidget(m_stack);

    connect(m_sidebar, &Sidebar::pageChanged, this, [this](int idx) {
        m_stack->setCurrentIndex(idx);
        refreshAllPages();
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

void MainWidget::refreshAllPages() {
    m_dashboard->refreshData();
    m_tickets->refreshData();
    m_subs->refreshData();
    m_reports->refreshData();
}

void MainWidget::onRefreshTimer() {
    // CORREGIDO: solo refresca la UI desde SQLite local.
    // No llamar sincronizarDesdeServidor() acá — el sync
    // solo debe ocurrir una vez al hacer login exitoso.
    m_sidebar->setSyncStatus(true, 0);
    refreshAllPages();
}

void MainWidget::onLogout() {

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Cerrar sesion",
        "¿Estas seguro de que queres cerrar sesion?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {

        DataManager::instance().cambiarEstadoRed(DataManager::SINCRONIZANDO);

        this->hide();

        LoginDialog login;

        if (login.exec() == QDialog::Accepted) {
            // CORREGIDO: no llamar sincronizarDesdeServidor() acá.
            // loginRed() dentro de LoginDialog ya lo dispara
            // automaticamente cuando el VPS responde OK.
            setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
            refreshAllPages();
            this->show();
        } else {
            qApp->quit();
        }
    }
}