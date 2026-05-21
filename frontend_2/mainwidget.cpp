#include "mainwidget.h"
#include "stylemanager.h"
#include "logindialog.h"
#include "datamanager.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>

/*
    ╔══════════════════════════════════════════════════════════════╗
    ║                        MainWidget.cpp                       ║
    ╠══════════════════════════════════════════════════════════════╣
    ║  Proposito:                                                 ║
    ║  Clase principal de la aplicacion AlcancIA.                 ║
    ║  Se encarga de administrar:                                 ║
    ║   • Sidebar de navegacion                                   ║
    ║   • Cambio entre paginas                                    ║
    ║   • Actualizacion automatica                                ║
    ║   • Logout y reinicio de sesion                             ║
    ╚══════════════════════════════════════════════════════════════╝
*/

/*
    ────────────────────────────────────────────────────────────────
    Constructor
    ────────────────────────────────────────────────────────────────
    Parametros:
        QWidget *parent
            → Widget padre de Qt (opcional).

    Funcion:
        • Configura ventana principal.
        • Aplica estilos globales.
        • Construye interfaz.
        • Inicia timer automatico de actualizacion.
*/
MainWidget::MainWidget(QWidget *parent) : QWidget(parent) {

    // Titulo principal de la ventana
    setWindowTitle("AlcancIA — Asistente Financiero");

    // Tamaño minimo permitido
    setMinimumSize(1100, 680);

    // Tamaño inicial de apertura
    resize(1280, 780);

    // Aplicar stylesheet general
    applyStyles();

    // Construir toda la UI
    setupUI();

    /*
        Timer automatico para refrescar datos.

        Intervalo:
            30000 ms = 30 segundos
    */
    m_refreshTimer = new QTimer(this);

    connect(m_refreshTimer,
            &QTimer::timeout,
            this,
            &MainWidget::onRefreshTimer);

    m_refreshTimer->start(30000);
}

/*
    ────────────────────────────────────────────────────────────────
    applyStyles()
    ────────────────────────────────────────────────────────────────
    Funcion:
        Aplica estilos globales a la aplicacion principal.
*/
void MainWidget::applyStyles() {

    setStyleSheet(
        StyleManager::appStyleSheet() + R"(

        QWidget {
            background-color: #0F1117;
        }

    )");
}

/*
    ────────────────────────────────────────────────────────────────
    setupUI()
    ────────────────────────────────────────────────────────────────
    Funcion:
        Construye toda la interfaz grafica principal.

    Componentes creados:
        • Sidebar
        • Area central
        • Stack de paginas
        • Dashboard
        • Tickets
        • Subscripciones
        • Reportes
*/
void MainWidget::setupUI() {

    // Layout principal horizontal
    QHBoxLayout *rootLayout = new QHBoxLayout(this);

    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    /*
        Sidebar lateral de navegacion.

        Contiene:
            • Botones de paginas
            • Logout
            • Estado de sincronizacion
    */
    m_sidebar = new Sidebar(this);

    // Conectar señal de logout
    connect(m_sidebar,
            &Sidebar::logoutRequested,
            this,
            &MainWidget::onLogout);

    /*
        Area de contenido principal
        donde se muestran las paginas.
    */
    QWidget *contentArea = new QWidget();

    contentArea->setStyleSheet(
        "background-color: #0F1117;"
        );

    QVBoxLayout *contentLayout =
        new QVBoxLayout(contentArea);

    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    /*
        Stack de paginas.

        Permite cambiar entre pantallas
        sin destruir widgets.
    */
    m_stack = new QStackedWidget();

    /*
        Creacion de paginas principales.
    */
    m_dashboard = new DashboardPage();
    m_tickets   = new TicketsPage();
    m_subs      = new SubscriptionsPage();
    m_reports   = new ReportsPage();

    /*
        Agregar paginas al stack.

        Indices:
            0 → Dashboard
            1 → Tickets
            2 → Subscripciones
            3 → Reportes
    */
    m_stack->addWidget(m_dashboard);
    m_stack->addWidget(m_tickets);
    m_stack->addWidget(m_subs);
    m_stack->addWidget(m_reports);

    // Insertar stack al area principal
    contentLayout->addWidget(m_stack);

    /*
        Cambio de pagina desde sidebar.
    */
    connect(m_sidebar,
            &Sidebar::pageChanged,
            this,
            [this](int idx) {

                // Cambiar pagina actual
                m_stack->setCurrentIndex(idx);

                // Refrescar datos
                refreshAllPages();
            });

    /*
        Navegacion rapida desde Dashboard
        hacia Tickets.
    */
    connect(m_dashboard,
            &DashboardPage::navigateToTickets,
            this,
            [this]() {

                m_sidebar->setActivePage(1);

                m_stack->setCurrentIndex(1);

                // Abrir carga automatica
                m_tickets->onUploadClicked();
            });

    /*
        Navegacion rapida desde Dashboard
        hacia Subscripciones.
    */
    connect(m_dashboard,
            &DashboardPage::navigateToSubs,
            this,
            [this]() {

                m_sidebar->setActivePage(2);

                m_stack->setCurrentIndex(2);

                // Abrir formulario de alta
                m_subs->onAddClicked();
            });

    /*
        Agregar widgets al layout raiz.
    */
    rootLayout->addWidget(m_sidebar);

    // El "1" indica expansion automatica
    rootLayout->addWidget(contentArea, 1);
}

/*
    ────────────────────────────────────────────────────────────────
    refreshAllPages()
    ────────────────────────────────────────────────────────────────
    Funcion:
        Actualiza todas las paginas del sistema.
*/
void MainWidget::refreshAllPages() {

    m_dashboard->refreshData();

    m_tickets->refreshData();

    m_subs->refreshData();

    m_reports->refreshData();
}

/*
    ────────────────────────────────────────────────────────────────
    onRefreshTimer()
    ────────────────────────────────────────────────────────────────
    Funcion:
        Ejecutada automaticamente cada 30 segundos.

    Uso:
        • Refrescar datos
        • Actualizar estado de sincronizacion
*/
void MainWidget::onRefreshTimer() {

    /*
        FUTURO:
            Aqui puede verificarse:
                • conexion a internet
                • sincronizacion cloud
                • pendientes offline
    */

    // Estado ficticio online
    m_sidebar->setSyncStatus(true, 0);

    // Refrescar contenido
    refreshAllPages();
}

/*
    ────────────────────────────────────────────────────────────────
    onLogout()
    ────────────────────────────────────────────────────────────────
    Funcion:
        Maneja cierre de sesion.

    Flujo:
        1. Confirmacion
        2. Guardar datos
        3. Ocultar ventana
        4. Mostrar Login
        5. Reabrir sesion o salir
*/
void MainWidget::onLogout() {

    QMessageBox::StandardButton reply;

    /*
        Mostrar confirmacion de logout.
    */
    reply = QMessageBox::question(
        this,
        "Cerrar sesion",
        "¿Estas seguro de que queres cerrar sesion?",
        QMessageBox::Yes | QMessageBox::No
        );

    // Si el usuario acepta
    if (reply == QMessageBox::Yes) {

        /*
            Guardar todos los datos antes
            de cerrar sesion.
        */
        DataManager::instance().saveToFile();

        // Ocultar ventana principal
        this->hide();

        /*
            Reabrir LoginDialog
            para nueva autenticacion.
        */
        LoginDialog login;

        if (login.exec() == QDialog::Accepted) {

            /*
                Usuario volvio a iniciar sesion.
            */

            // Actualizar titulo con usuario
            setWindowTitle(
                QString("AlcancIA - %1")
                    .arg(login.getUsername())
                );

            // Refrescar informacion
            refreshAllPages();

            // Mostrar nuevamente ventana
            this->show();

        } else {

            /*
                Usuario cancelo login.
                Cerrar aplicacion completa.
            */
            qApp->quit();
        }
    }
}