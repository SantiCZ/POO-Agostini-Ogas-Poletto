#include "mainwidget.h"
#include "stylemanager.h"
#include "logindialog.h"
#include "datamanager.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

#include <QPushButton>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent) {

    setWindowTitle("AlcancIA — Asistente Financiero");
    setMinimumSize(1100, 680);
    resize(1280, 780);

    applyStyles();
    setupUI();

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWidget::onRefreshTimer);
    m_refreshTimer->start(30000);
}

void MainWidget::applyStyles() {
    setStyleSheet(
        StyleManager::appStyleSheet() + R"(
        QWidget { background-color: #0F1117; }
    )");
}

void MainWidget::setupUI() {

    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_sidebar = new Sidebar(this);
    connect(m_sidebar, &Sidebar::logoutRequested, this, &MainWidget::onLogout);
    connect(m_sidebar, &Sidebar::uploadTicketRequested, this, [this]() {
        m_sidebar->setActivePage(1);
        m_stack->setCurrentIndex(1);
        m_tickets->onUploadClicked();
    });

    // ── Área de contenido ───────────────────────────────────────
    QWidget *contentArea = new QWidget();
    contentArea->setStyleSheet("background-color: #0F1117;");

    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // ── Topbar con campanita ────────────────────────────────────
    QWidget *topbar = new QWidget();
    topbar->setFixedHeight(52);
    topbar->setStyleSheet(
        "background-color: #0F1117;"
        "border-bottom: 1px solid #1E2235;"
        );

    QHBoxLayout *topbarL = new QHBoxLayout(topbar);
    topbarL->setContentsMargins(20, 0, 20, 0);
    topbarL->addStretch();

    // Contenedor campanita + badge (posicionamiento relativo)
    QWidget *notifContainer = new QWidget();
    notifContainer->setFixedSize(38, 38);
    notifContainer->setStyleSheet("background: transparent;");

    m_notifBtn = new QPushButton("🔔", notifContainer);
    m_notifBtn->setFixedSize(36, 36);
    m_notifBtn->setCursor(Qt::PointingHandCursor);
    m_notifBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 10px;
            font-size: 16px;
        }
        QPushButton:hover { background-color: #21253A; border-color: #38BDF8; }
    )");
    connect(m_notifBtn, &QPushButton::clicked, this, &MainWidget::onNotificacionesClicked);

    // Badge rojo (número)
    m_notifBadge = new QLabel("0", notifContainer);
    m_notifBadge->setFixedSize(16, 16);
    m_notifBadge->move(22, 0);   // esquina superior derecha
    m_notifBadge->setAlignment(Qt::AlignCenter);
    m_notifBadge->setStyleSheet(R"(
        QLabel {
            background-color: #F87171;
            color: white;
            border-radius: 8px;
            font-size: 9px;
            font-weight: 700;
        }
    )");
    m_notifBadge->hide();

    //borar despues
    QPushButton *btnProbarSync = new QPushButton("Probar Sync");
    btnProbarSync->setCursor(Qt::PointingHandCursor);

    connect(btnProbarSync, &QPushButton::clicked, this, []()
            {
                qDebug() << "Boton Probar Sync presionado";

                DataManager::instance().renovarSuscripcionesVencidasLocales();
                DataManager::instance().sincronizarSuscripcionesLocales();
            });

    topbarL->addWidget(btnProbarSync);
    //-------------------------------------------------------------

    topbarL->addWidget(notifContainer);

    contentLayout->addWidget(topbar);

    // ── Stack de páginas ────────────────────────────────────────
    m_stack = new QStackedWidget();

    m_dashboard = new DashboardPage();
    m_tickets   = new TicketsPage();
    m_subs      = new SubscriptionsPage();
    m_reports   = new ReportsPage();

    m_stack->addWidget(m_dashboard); // 0
    m_stack->addWidget(m_tickets);   // 1
    m_stack->addWidget(m_subs);      // 2
    m_stack->addWidget(m_reports);   // 3

    contentLayout->addWidget(m_stack, 1);

    // ── Panel flotante de notificaciones ───────────────────────
    m_notifPanel = buildNotifPanel();
    m_notifPanel->setParent(contentArea);   // hijo de contentArea para posicionarlo
    m_notifPanel->hide();

    connect(m_sidebar, &Sidebar::pageChanged, this, [this](int idx) {
        m_stack->setCurrentIndex(idx);
        refreshAllPages();
    });

    connect(&DataManager::instance(), &DataManager::sincronizacionCompletada,
            this, [this]() {
                refreshAllPages();
                updateNotifBadge();
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

    // Badge inicial
    updateNotifBadge();
}

// ── Panel flotante ──────────────────────────────────────────────
QWidget* MainWidget::buildNotifPanel()
{
    QWidget *panel = new QWidget();
    panel->setFixedWidth(320);
    panel->setStyleSheet(R"(
        QWidget {
            background-color: #13151F;
            border: 1px solid #2E3347;
            border-radius: 14px;
        }
    )");

    QVBoxLayout *panelL = new QVBoxLayout(panel);
    panelL->setContentsMargins(0, 0, 0, 0);
    panelL->setSpacing(0);

    // Header del panel
    QWidget *header = new QWidget();
    header->setStyleSheet("background: transparent; border-bottom: 1px solid #2E3347;");
    QHBoxLayout *headerL = new QHBoxLayout(header);
    headerL->setContentsMargins(16, 12, 16, 12);

    QLabel *titleLbl = new QLabel("🔔  Notificaciones");
    titleLbl->setStyleSheet(
        "color: #F1F5F9; font-size: 14px; font-weight: 700; border: none;"
        );

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(24, 24);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #64748B; border: none; font-size: 13px; }"
        "QPushButton:hover { color: #F1F5F9; }"
        );
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        m_notifPanel->hide();
        m_notifVisible = false;
    });

    headerL->addWidget(titleLbl);
    headerL->addStretch();
    headerL->addWidget(closeBtn);
    panelL->addWidget(header);

    // Scroll con lista
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget *listW = new QWidget();
    listW->setStyleSheet("background: transparent;");
    m_notifListLayout = new QVBoxLayout(listW);
    m_notifListLayout->setContentsMargins(12, 8, 12, 8);
    m_notifListLayout->setSpacing(6);
    m_notifListLayout->addStretch();

    scroll->setWidget(listW);
    panelL->addWidget(scroll);

    return panel;
}

// ── Actualizar badge y lista ─────────────────────────────────────
// void MainWidget::updateNotifBadge()
// {
//     // QVector<Notificacion> notifs = DataManager::instance().getNotificaciones();
//     QVector<Notificacion> notifs =
//         DataManager::instance().getNotificaciones();

//     int count = notifs.size();
//     if (count > 0) {
//         m_notifBadge->setText(QString::number(count));
//         // m_notifBadge->show();
//         m_notifBadge->setVisible(count > 0);
//     } else {
//         m_notifBadge->hide();
//     }

//     // Reconstruir lista dentro del panel
//     while (QLayoutItem *item = m_notifListLayout->takeAt(0)) {
//         if (item->widget()) delete item->widget();
//         delete item;
//     }

//     if (notifs.isEmpty()) {
//         QLabel *empty = new QLabel("✅  Sin notificaciones pendientes");
//         empty->setAlignment(Qt::AlignCenter);
//         empty->setStyleSheet(
//             "color: #475569; font-size: 13px; padding: 24px; background: transparent;"
//             );
//         m_notifListLayout->addWidget(empty);
//     } else {
//         for (const Notificacion &n : notifs) {
//             // QFrame *card = new QFrame();
//             QLabel *fechaL = new QLabel(n.fechaTexto);
//             card->setStyleSheet(R"(
//                 QFrame {
//                     background-color: rgba(251,191,36,0.06);
//                     border: 1px solid rgba(251,191,36,0.2);
//                     border-radius: 10px;
//                 }
//             )");
//             QVBoxLayout *cardL = new QVBoxLayout(card);
//             cardL->setContentsMargins(12, 10, 12, 10);
//             cardL->setSpacing(3);

//             QLabel *msgL = new QLabel("⚠  " + n.mensaje);
//             msgL->setWordWrap(true);
//             msgL->setStyleSheet(
//                 "color: #FBBF24; font-size: 12px; font-weight: 600; background: transparent;"
//                 );

//             QLabel *fechaL = new QLabel(n.fecha.toString("dd/MM/yyyy"));
//             fechaL->setStyleSheet(
//                 "color: #64748B; font-size: 11px; background: transparent;"
//                 );

//             cardL->addWidget(msgL);
//             cardL->addWidget(fechaL);
//             m_notifListLayout->addWidget(card);
//          }
//     }
//     m_notifListLayout->addStretch();
// }

void MainWidget::updateNotifBadge()
{
    QVector<Notificacion> notifs = DataManager::instance().getNotificaciones();
    int count = notifs.size();

    if (count > 0) {
        m_notifBadge->setText(QString::number(count));
        m_notifBadge->setVisible(true);
    } else {
        m_notifBadge->setVisible(false);
    }

    // Limpiar layout del panel
    while (QLayoutItem *item = m_notifListLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    if (notifs.isEmpty()) {
        QLabel *empty = new QLabel("✅  Sin notificaciones pendientes");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #475569; font-size: 13px; padding: 24px; background: transparent;");
        m_notifListLayout->addWidget(empty);
    } else {
        for (const Notificacion &n : notifs) {
            QFrame *card = new QFrame();   // <--- declaración correcta
            card->setStyleSheet(R"(
                QFrame {
                    background-color: rgba(251,191,36,0.06);
                    border: 1px solid rgba(251,191,36,0.2);
                    border-radius: 10px;
                }
            )");
            QVBoxLayout *cardL = new QVBoxLayout(card);
            cardL->setContentsMargins(12, 10, 12, 10);
            cardL->setSpacing(3);

            QLabel *msgL = new QLabel("⚠  " + n.mensaje);
            msgL->setWordWrap(true);
            msgL->setStyleSheet("color: #FBBF24; font-size: 12px; font-weight: 600; background: transparent;");

            QLabel *fechaL = new QLabel(n.fecha.toString("dd/MM/yyyy"));
            fechaL->setStyleSheet("color: #64748B; font-size: 11px; background: transparent;");

            cardL->addWidget(msgL);
            cardL->addWidget(fechaL);
            m_notifListLayout->addWidget(card);
        }
    }
    m_notifListLayout->addStretch();
}
// ── Toggle panel notificaciones ─────────────────────────────────
void MainWidget::onNotificacionesClicked()
{
    m_notifVisible = !m_notifVisible;

    if (m_notifVisible) {
        updateNotifBadge();

        // Posicionar panel: esquina superior derecha del contentArea
        QWidget *contentArea = qobject_cast<QWidget*>(m_notifPanel->parent());
        int panelH = qMin(400, 80 + 70 * qMax(1, (int)DataManager::instance().getNotificaciones().size()));
        m_notifPanel->setFixedHeight(panelH);
        int x = contentArea->width() - m_notifPanel->width() - 12;
        m_notifPanel->move(x, 54);
        m_notifPanel->raise();
        m_notifPanel->show();
    } else {
        m_notifPanel->hide();
    }
}

void MainWidget::refreshAllPages() {
    m_dashboard->refreshData();
    m_tickets->refreshData();
    m_subs->refreshData();
    m_reports->refreshData();
}

void MainWidget::onRefreshTimer() {
    m_sidebar->setSyncStatus(true, 0);
    refreshAllPages();
    updateNotifBadge();
}

void MainWidget::onLogout() {

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Cerrar sesion",
        "¿Estas seguro de que queres cerrar sesion?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        DataManager::instance().cambiarEstadoRed(DataManager::SINCRONIZANDO);
        this->hide();

        LoginDialog login;
        if (login.exec() == QDialog::Accepted) {
            setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
            refreshAllPages();
            updateNotifBadge();
            this->show();
        } else {
            qApp->quit();
        }
    }
}